#include <pch.hpp>
#include "Graphics.hpp"

#include <Core/src/utils/VulkanHelper.hpp>
#include <Core/src/utils/FileLoader.hpp>

#include "gfx.hpp"
#include "Exception.hpp"



using namespace std::string_literals;


namespace gage::gfx
{
    const std::vector<const char *> Graphics::ENABLED_INSTANCE_EXTENSIONS 
    {

    };

    const std::vector<const char *> Graphics::ENABLED_DEVICE_EXTENSIONS
    {

    };

    Graphics::Graphics(GLFWwindow *window, uint32_t width, uint32_t height, std::string app_name) : 
        app_name{std::move(app_name)},
        draw_extent{width, height},
        draw_extent_temp{width, height},
        instance(app_name, window),
        device(instance),
        swapchain(instance, device, draw_extent),
        desc_pool(device),
        global_desc_layout(device),
        cmd_pool(device),
        allocator(device, instance),
        defaults(device, cmd_pool, allocator),
        frame_datas { 
            data::FrameData(device, desc_pool, cmd_pool, allocator.allocator, global_desc_layout.layout), 
            data::FrameData(device, desc_pool, cmd_pool, allocator.allocator, global_desc_layout.layout)
        },
        directional_light_shadow_map_resolution(2048),
        directional_light_shadow_map_resolution_temp(2048),
        geometry_buffer(*this),
        final_ambient(*this),
        directional_light(*this),
        point_light(*this),
        ssao(*this)
    {

    }

    Graphics::~Graphics()
    {
       
    }

    void Graphics::wait()
    {
        vkDeviceWaitIdle(device.device);
    }

    VkCommandBuffer Graphics::clear(const data::Camera &camera)
    {
        VkSemaphore &present_semaphore = frame_datas[frame_index].present_semaphore;
        VkFence &render_fence = frame_datas[frame_index].render_fence;
        VmaAllocationInfo &global_alloc_info = frame_datas[frame_index].global_alloc_info;

        // wait until the GPU has finished rendering the last frame. Timeout of 1 second
        vk_check(vkWaitForFences(device.device, 1, &render_fence, true, 1000000000));
        vk_check(vkResetFences(device.device, 1, &render_fence));

        // Check for swapchain recreation
        if (resize_requested)
        {
            vkDeviceWaitIdle(device.device);
            draw_extent = draw_extent_temp;
            swapchain.reset();
            geometry_buffer.reset();
            final_ambient.reset();
            directional_light.reset();
            point_light.reset();
            ssao.reset();
            resize_requested = false;
        }

        if (directional_light_shadow_map_resolution != directional_light_shadow_map_resolution_temp)
        {
            vkDeviceWaitIdle(device.device);
            directional_light_shadow_map_resolution = directional_light_shadow_map_resolution_temp;
            geometry_buffer.reset_shadowmap();
        }

        // Update global set of currnet frame
        global_uniform.camera_position = camera.get_position();
        global_uniform.projection = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                                                             (float)draw_extent.width, (float)draw_extent.height, camera.get_near(), camera.get_far());
        global_uniform.projection[1][1] *= -1;
        global_uniform.inv_projection = glm::inverse(global_uniform.projection);
        global_uniform.view = camera.get_view();
        global_uniform.inv_view = glm::inverse(global_uniform.view);
        global_uniform.directional_light_proj_views[0] = calculate_directional_light_proj_view(camera, 0.1f, global_uniform.directional_light_cascade_planes[0].x);
        global_uniform.directional_light_proj_views[1] = calculate_directional_light_proj_view(camera, 0.1f, global_uniform.directional_light_cascade_planes[1].x);
        global_uniform.directional_light_proj_views[2] = calculate_directional_light_proj_view(camera, 0.1f, global_uniform.directional_light_cascade_planes[2].x);

        std::memcpy(global_alloc_info.pMappedData, &global_uniform, sizeof(GlobalUniform));

        if (VkResult result = vkAcquireNextImageKHR(device.device, swapchain.swapchain, 1000000000, present_semaphore, nullptr, &swapchain.image_index);
            result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            log().critical("Failed to acquire next image: {}", string_VkResult(result));
            throw GraphicsException{};
        }

        auto &cmd = frame_datas[frame_index].cmd;
        // reset the command buffer
        vk_check(vkResetCommandBuffer(cmd, 0));
        // begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
        VkCommandBufferBeginInfo cmd_begin_info = {};
        cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vk_check(vkBeginCommandBuffer(cmd, &cmd_begin_info));

        return cmd;
    }

    void Graphics::end_frame(VkCommandBuffer cmd)
    {

        VkSemaphore &present_semaphore = frame_datas[frame_index].present_semaphore;
        VkSemaphore &render_semaphore = frame_datas[frame_index].render_semaphore;
        VkFence &render_fence = frame_datas[frame_index].render_fence;

        VkImageMemoryBarrier swapchain_memory_barrier{};
        swapchain_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        swapchain_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        swapchain_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // Blit swapchain color image to swapchain image and wait for color result
        swapchain_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        swapchain_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        swapchain_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        swapchain_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        swapchain_memory_barrier.image = swapchain.at(swapchain.image_index);
        swapchain_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapchain_memory_barrier.subresourceRange.baseMipLevel = 0;
        swapchain_memory_barrier.subresourceRange.levelCount = 1;
        swapchain_memory_barrier.subresourceRange.baseArrayLayer = 0;
        swapchain_memory_barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &swapchain_memory_barrier);

        VkImageBlit region{};
        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.mipLevel = 0;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = 1;
        region.srcOffsets[0].x = 0;
        region.srcOffsets[0].y = 0;
        region.srcOffsets[0].z = 0;
        region.srcOffsets[1].x = get_scaled_draw_extent().width;
        region.srcOffsets[1].y = get_scaled_draw_extent().height;
        region.srcOffsets[1].z = 1;
        region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstSubresource.mipLevel = 0;
        region.dstSubresource.baseArrayLayer = 0;
        region.dstSubresource.layerCount = 1;
        region.dstOffsets[0].x = 0;
        region.dstOffsets[0].y = 0;
        region.dstOffsets[0].z = 0;
        region.dstOffsets[1].x = draw_extent.width;
        region.dstOffsets[1].y = draw_extent.height;
        region.dstOffsets[1].z = 1;

        vkCmdBlitImage(cmd, geometry_buffer.get_final_color(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       swapchain.at(swapchain.image_index), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &region, VK_FILTER_NEAREST);

        swapchain_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        swapchain_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        swapchain_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        swapchain_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        swapchain_memory_barrier.image = swapchain.at(swapchain.image_index);
        swapchain_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapchain_memory_barrier.subresourceRange.baseMipLevel = 0;
        swapchain_memory_barrier.subresourceRange.levelCount = 1;
        swapchain_memory_barrier.subresourceRange.baseArrayLayer = 0;
        swapchain_memory_barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &swapchain_memory_barrier);
        vk_check(vkEndCommandBuffer(cmd));

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        std::array<VkSemaphore, 1> wait_semaphores = {present_semaphore};
        submit.waitSemaphoreCount = wait_semaphores.size();
        submit.pWaitSemaphores = wait_semaphores.data();
        submit.pWaitDstStageMask = wait_stages;

        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &render_semaphore;

        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        vk_check(vkQueueSubmit(device.queue, 1, &submit, render_fence));

        VkSwapchainKHR swapchain = this->swapchain.swapchain;
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &render_semaphore;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &this->swapchain.image_index;

        if (VkResult result = vkQueuePresentKHR(device.queue, &presentInfo);
            result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            log().critical("Failed to present swapchain image: {}", string_VkResult(result));
            throw GraphicsException{};
        }

        frame_index = (frame_index + 1) % FRAMES_IN_FLIGHT;
    }



    glm::mat4x4 Graphics::calculate_directional_light_proj_view(const data::Camera &camera, float near, float far)
    {
        glm::vec4 frustum_corners[] =
            {
                {-1, -1, 0, 1},
                {1, -1, 0, 1},
                {-1, 1, 0, 1},
                {1, 1, 0, 1},

                {-1, -1, 1, 1},
                {1, -1, 1, 1},
                {-1, 1, 1, 1},
                {1, 1, 1, 1},
            };
        glm::mat4x4 cam_proj = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                                                        (float)draw_extent.width, (float)draw_extent.height, near, far);
        //cam_proj[1][1] *= -1;

        glm::mat4x4 inv_proj = glm::inverse(cam_proj * global_uniform.view);

        glm::vec3 center{0, 0, 0};
        for (size_t i = 0; i < 8; i++)
        {
            frustum_corners[i] = inv_proj * frustum_corners[i];
            frustum_corners[i] /= frustum_corners[i].w;
            center += glm::vec3(frustum_corners[i]);
        }
        center /= 8.0;

        glm::mat4 light_view = glm::lookAt(center - global_uniform.directional_light_direction,
                                           center,
                                           glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec3 min{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()};

        glm::vec3 max{
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest()};
        for (size_t i = 0; i < 8; i++)
        {
            glm::vec4 frustum_corner = light_view * frustum_corners[i];
            min.x = glm::min(min.x, frustum_corner.x);
            min.y = glm::min(min.y, frustum_corner.y);
            min.z = glm::min(min.z, frustum_corner.z);

            max.x = glm::max(max.x, frustum_corner.x);
            max.y = glm::max(max.y, frustum_corner.y);
            max.z = glm::max(max.z, frustum_corner.z);
        }

        constexpr float zMult = 5.0f;
        if (min.z < 0)
        {
            min.z *= zMult;
        }
        else
        {
            min.z /= zMult;
        }
        if (max.z < 0)
        {
            max.z /= zMult;
        }
        else
        {
            max.z *= zMult;
        }

        return glm::orthoRH_ZO(min.x, max.x, min.y, max.y, min.z, max.z) * light_view;
    }

    void Graphics::resize(int width, int height)
    {
        draw_extent_temp.width = width;
        draw_extent_temp.height = height;
        resize_requested = true;
    }
    void Graphics::resize_shadow_map(uint32_t shadow_map_size)
    {
        directional_light_shadow_map_resolution_temp = shadow_map_size;
    }

    
    VkExtent2D Graphics::get_scaled_draw_extent() const
    {
        return VkExtent2D{(unsigned int)std::floor(draw_extent.width * draw_extent_scale),
                          (unsigned int)std::floor(draw_extent.height * draw_extent_scale)};
    }

}
