#include <pch.hpp>
#include "Graphics.hpp"

#include "gfx.hpp"
#include <Core/src/utils/VulkanHelper.hpp>
#include <Core/src/utils/FileLoader.hpp>

#include "Exception.hpp"

#include "data/g_buffer/GBuffer.hpp"
#include "data/Camera.hpp"
#include "data/PBRPipeline.hpp"
#include "data/AmbientLight.hpp"
#include "data/DirectionalLight.hpp"
#include "data/ShadowPipeline.hpp"
#include "data/PointLight.hpp"
#include "data/SSAO.hpp"

#include "data/terrain/TerrainPipeline.hpp"

using namespace std::string_literals;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
    void *p_user_data)
{
    using namespace gage;
    gfx::Graphics *gfx = (gfx::Graphics *)p_user_data;
    switch (message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    {
        gfx::log().info("{}", gfx->get_app_name());
        gfx::log().trace("{}", p_callback_data->pMessage);

        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        gfx::log().info("{}", p_callback_data->pMessage);
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        gfx::log().warn("{}", p_callback_data->pMessage);
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        utils::StackTrace stack_trace;
        gfx::log().error("{}\n{}", p_callback_data->pMessage, stack_trace.print());

        break;
    }

    default:
        break;
    }

    return VK_FALSE;
}

namespace gage::gfx
{
    Graphics::Graphics(GLFWwindow *window, std::string app_name) : app_name{std::move(app_name)}
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        draw_extent.width = width;
        draw_extent.height = height;
        draw_extent_temp.width = width;
        draw_extent_temp.height = height;

        vkb::InstanceBuilder builder;
        // make the vulkan instance, with basic debug features
        auto vkb_instance_result = builder.set_app_name(app_name.c_str())
                                       .request_validation_layers(true)
                                       .set_debug_callback(debugCallback)
                                       .set_debug_callback_user_data_pointer(this)
                                       .enable_extensions(ENABLED_INSTANCE_EXTENSIONS.size(), ENABLED_INSTANCE_EXTENSIONS.data())
                                       .require_api_version(1, 3, 0)
                                       .build();

        vkb_check(vkb_instance_result, "Failed to create vulkan instance !"s);

        vkb::Instance vkb_inst = vkb_instance_result.value();

        // grab the instance
        instance = vkb_inst.instance;
        debug_messenger = vkb_inst.debug_messenger;

        delete_stack.push([this]()
                          {
            vkb::destroy_debug_utils_messenger(instance, debug_messenger);
		    vkDestroyInstance(instance, nullptr); });

        vk_check(glfwCreateWindowSurface(instance, window, nullptr, &surface));
        delete_stack.push([this]()
                          { vkDestroySurfaceKHR(instance, surface, nullptr); });

        // vulkan 1.2 features
        // VkPhysicalDeviceFeatures features = {};
        // features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        // features.ver
        // features12.ver

        // vulkan 1.3 features
        VkPhysicalDeviceFeatures features{};
        features.geometryShader = true;

        VkPhysicalDeviceVulkan13Features features13 = {};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.dynamicRendering = true;
        features13.synchronization2 = true;

        // use vkbootstrap to select a gpu.
        vkb::PhysicalDeviceSelector selector{vkb_inst};
        auto physical_device_result = selector
                                          .set_minimum_version(1, 3)
                                          .set_required_features(features)
                                          .set_required_features_13(features13)
                                          .add_required_extensions(ENABLED_DEVICE_EXTENSIONS.size(), ENABLED_DEVICE_EXTENSIONS.data())
                                          .set_surface(surface)
                                          .select();

        vkb_check(physical_device_result, "Failed to select physical device: ");

        vkb::PhysicalDevice vkb_physical_device = physical_device_result.value();
        log().info("Selected physical device: " + vkb_physical_device.name);

        vkb::DeviceBuilder deviceBuilder{vkb_physical_device};
        auto vkb_device_result = deviceBuilder.build();
        vkb_check(vkb_device_result, "Failed to create device: ");

        auto vkb_device = vkb_device_result.value();

        device = vkb_device.device;
        physical_device = vkb_physical_device.physical_device;
        delete_stack.push([this]()
                          { vkDestroyDevice(device, nullptr); });

        // Create allocator
        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        // allocatorCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorCreateInfo.physicalDevice = physical_device;
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.instance = instance;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        vmaCreateAllocator(&allocatorCreateInfo, &allocator);
        delete_stack.push([this]()
                          { vmaDestroyAllocator(allocator); });

        swapchain.emplace(*this);
        delete_stack.push([this]()
                          { swapchain.reset(); });

        // Create descriptor pool
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
        };

        // Create descriptor pool
        VkDescriptorPoolCreateInfo desc_pool_ci{};
        desc_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_ci.maxSets = 4;
        desc_pool_ci.pPoolSizes = pool_sizes;
        desc_pool_ci.poolSizeCount = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
        desc_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        vkCreateDescriptorPool(device, &desc_pool_ci, nullptr, &desc_pool);
        delete_stack.push([this]()
                          { vkDestroyDescriptorPool(device, desc_pool, nullptr); });

        // Define a global set layout
        //  GLOBAL SET LAYOUT
        VkDescriptorSetLayoutBinding global_bindings[] = {
            {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL, .pImmutableSamplers = nullptr},
            {.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL, .pImmutableSamplers = nullptr},
        };

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = sizeof(global_bindings) / sizeof(global_bindings[0]);
        layout_ci.pBindings = global_bindings;
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(device, &layout_ci, nullptr, &global_set_layout));
        delete_stack.push([this]()
                          { vkDestroyDescriptorSetLayout(device, global_set_layout, nullptr); });

        // use vkbootstrap to get a Graphics queue family
        auto queue_family_result = vkb_device.get_queue_index(vkb::QueueType::graphics);
        vkb_check(queue_family_result);
        queue_family = queue_family_result.value();
        // Get queue
        vkGetDeviceQueue(device, queue_family, 0, &queue);

        // Create command pool
        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_info.queueFamilyIndex = queue_family;
        vk_check(vkCreateCommandPool(device, &command_pool_info, nullptr, &cmd_pool));
        delete_stack.push([this]()
                          { vkDestroyCommandPool(device, cmd_pool, nullptr); });

        // Create Frame datas
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            FrameData &data = frame_datas[i];
            VkFence &render_fence = data.render_fence;
            VkSemaphore &present_semaphore = data.present_semaphore;
            VkSemaphore &render_semaphore = data.render_semaphore;
            VkCommandBuffer &cmd = data.cmd;
            VkDescriptorSet &global_set = data.global_set;
            VkBuffer &global_buffer = data.global_buffer;
            VmaAllocation &global_alloc = data.global_alloc;
            VmaAllocationInfo &global_alloc_info = data.global_alloc_info;

            VkFenceCreateInfo fenceCreateInfo = {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            // we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            vk_check(vkCreateFence(device, &fenceCreateInfo, nullptr, &render_fence));
            delete_stack.push([this, render_fence]()
                              { vkDestroyFence(device, render_fence, nullptr); });

            // for the semaphores we don't need any flags
            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.flags = 0;

            vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &present_semaphore));
            vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &render_semaphore));

            // Allocate command buffer
            VkCommandBufferAllocateInfo cmd_alloc_info = {};
            cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd_alloc_info.commandPool = cmd_pool;
            cmd_alloc_info.commandBufferCount = 1;

            vk_check(vkAllocateCommandBuffers(device, &cmd_alloc_info, &cmd));
            delete_stack.push([this, cmd]()
                              { vkFreeCommandBuffers(device, cmd_pool, 1, &cmd); });

            delete_stack.push([this, present_semaphore, render_semaphore]()
                              {
                vkDestroySemaphore(device, present_semaphore, nullptr);
                vkDestroySemaphore(device, render_semaphore, nullptr); });

            // Create global uniform buffer
            VkBufferCreateInfo buffer_ci = {};
            buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_ci.size = sizeof(GlobalUniform);
            buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            VmaAllocationCreateInfo alloc_ci = {};
            alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
            alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            vk_check(vmaCreateBuffer(allocator, &buffer_ci, &alloc_ci, &global_buffer, &global_alloc, &global_alloc_info));

            delete_stack.push([&]()
                              { vmaDestroyBuffer(allocator, global_buffer, global_alloc); });

            // Allocate descriptor set
            VkDescriptorSetAllocateInfo global_set_alloc_info{};
            global_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            global_set_alloc_info.descriptorPool = desc_pool;
            global_set_alloc_info.descriptorSetCount = 1;
            global_set_alloc_info.pSetLayouts = &global_set_layout;
            vkAllocateDescriptorSets(device, &global_set_alloc_info, &global_set);
            delete_stack.push([&, this]()
                              { vkFreeDescriptorSets(device, desc_pool, 1, &global_set); });

            // Link global uniform to globlal set
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = global_buffer;
            buffer_info.offset = 0;
            buffer_info.range = sizeof(GlobalUniform);

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write.dstBinding = 0;
            descriptor_write.dstSet = global_set;
            descriptor_write.pBufferInfo = &buffer_info;
            vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);
        }
        // Create default datas
        create_default_image_sampler();
        delete_stack.push([this]()
                          { destroy_default_image_sampler(); });

        geometry_buffer = std::make_unique<data::g_buffer::GBuffer>(*this);
        delete_stack.push([this]()
                          { geometry_buffer.reset(); });

        shadow_pipeline = std::make_unique<data::ShadowPipeline>(*this);
        delete_stack.push([this]()
                          { shadow_pipeline.reset(); });

        pbr_pipeline = std::make_unique<data::PBRPipeline>(*this);
        delete_stack.push([this]()
                          { pbr_pipeline.reset(); });

        final_ambient = std::make_unique<data::AmbientLight>(*this);
        delete_stack.push([this]()
                          { final_ambient.reset(); });

        directional_light = std::make_unique<data::DirectionalLight>(*this);
        delete_stack.push([this]()
                          { directional_light.reset(); });

        point_light = std::make_unique<data::PointLight>(*this);
        delete_stack.push([this]()
                          { point_light.reset(); });

        terrain_pipeline = std::make_unique<data::terrain::TerrainPipeline>(*this);
        delete_stack.push([this]()
                          { terrain_pipeline.reset(); });

        ssao = std::make_unique<data::SSAO>(*this);
        delete_stack.push([this]()
                          { ssao.reset(); });
    }

    Graphics::~Graphics()
    {
        // Xoa object nguoc lai
        while (!delete_stack.empty())
        {
            delete_stack.top()(); // call the delete function
            delete_stack.pop();   // pop the stack
        }
    }

    void Graphics::wait()
    {
        vkDeviceWaitIdle(device);
    }

    // }

    VkCommandBuffer Graphics::clear(const data::Camera &camera)
    {
        uploading_mutex.lock();

        VkSemaphore &present_semaphore = frame_datas[frame_index].present_semaphore;
        VkFence &render_fence = frame_datas[frame_index].render_fence;
        VmaAllocationInfo &global_alloc_info = frame_datas[frame_index].global_alloc_info;

        // wait until the GPU has finished rendering the last frame. Timeout of 1 second
        vk_check(vkWaitForFences(device, 1, &render_fence, true, 1000000000));
        vk_check(vkResetFences(device, 1, &render_fence));

        // Check for swapchain recreation
        if (resize_requested)
        {
            vkDeviceWaitIdle(device);
            draw_extent = draw_extent_temp;
            swapchain.reset();
            swapchain.emplace(*this);
            geometry_buffer->reset();
            final_ambient->reset();
            directional_light->reset();
            point_light->reset();
            ssao->reset();
            resize_requested = false;
        }

        if (directional_light_shadow_map_resize_requested)
        {
            vkDeviceWaitIdle(device);
            directional_light_shadow_map_resolution = directional_light_shadow_map_resolution_temp;
            geometry_buffer->reset_shadowmap();
            directional_light_shadow_map_resize_requested = false;
        }

        // Update global set of currnet frame
        global_uniform.camera_position = camera.get_position();
        global_uniform.projection = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                                                             (float)draw_extent.width, (float)draw_extent.height, camera.get_near(), camera.get_far());
        global_uniform.projection[1][1] *= -1;
        global_uniform.view = camera.get_view();
        global_uniform.directional_light_proj_views[0] = calculate_directional_light_proj_view(camera, 0.1f, global_uniform.directional_light_cascade_planes[0].x);
        global_uniform.directional_light_proj_views[1] = calculate_directional_light_proj_view(camera, 0.1f, global_uniform.directional_light_cascade_planes[1].x);
        global_uniform.directional_light_proj_views[2] = calculate_directional_light_proj_view(camera, 0.1f, global_uniform.directional_light_cascade_planes[2].x);

        std::memcpy(global_alloc_info.pMappedData, &global_uniform, sizeof(GlobalUniform));

        if (VkResult result = vkAcquireNextImageKHR(device, swapchain->get(), 1000000000, present_semaphore, nullptr, &swapchain_image_index);
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

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, final_ambient->get_layout(), 0, 1, &frame_datas[frame_index].global_set, 0, nullptr);

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
        swapchain_memory_barrier.image = swapchain->at(swapchain_image_index);
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

        vkCmdBlitImage(cmd, geometry_buffer->get_final_color(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       swapchain->at(swapchain_image_index), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &region, VK_FILTER_NEAREST);

        swapchain_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        swapchain_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        swapchain_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        swapchain_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        swapchain_memory_barrier.image = swapchain->at(swapchain_image_index);
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

        std::array<VkSemaphore, 1> wait_semaphores = {
            present_semaphore};
        submit.waitSemaphoreCount = wait_semaphores.size();
        submit.pWaitSemaphores = wait_semaphores.data();
        submit.pWaitDstStageMask = wait_stages;

        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &render_semaphore;

        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        vk_check(vkQueueSubmit(queue, 1, &submit, render_fence));

        VkSwapchainKHR swapchain = this->swapchain->get();
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &render_semaphore;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &swapchain_image_index;

        if (VkResult result = vkQueuePresentKHR(queue, &presentInfo);
            result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            log().critical("Failed to present swapchain image: {}", string_VkResult(result));
            throw GraphicsException{};
        }

        frame_index = (frame_index + 1) % FRAMES_IN_FLIGHT;

        uploading_mutex.unlock();
    }

    Graphics::GlobalUniform &Graphics::get_global_uniform()
    {
        return global_uniform;
    }

    const std::string &Graphics::get_app_name() const noexcept
    {
        return app_name;
    }

    void Graphics::set_resolution_scale(float scale)
    {
        assert(scale >= 0.0f && scale <= 2.0f);
        draw_extent_scale = scale;
    }

    void Graphics::create_default_image_sampler()
    {
        // Create default sampler
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_NEAREST;
        sampler_info.minFilter = VK_FILTER_NEAREST;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.anisotropyEnable = VK_FALSE;
        sampler_info.maxAnisotropy = 0;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = 0.0f;

        vk_check(vkCreateSampler(device, &sampler_info, nullptr, &default_sampler));

        // Create default image and image view

        unsigned char image_data[] =
            {
                255, 255, 255, 255, 255, 255, 255, 255, 255,
                255, 255, 255, 255, 255, 255, 255, 255};
        VkImageCreateInfo img_ci = {};
        img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        img_ci.imageType = VK_IMAGE_TYPE_2D;
        img_ci.extent.width = 2;
        img_ci.extent.height = 2;
        img_ci.extent.depth = 1;
        img_ci.mipLevels = 1;
        img_ci.arrayLayers = 1;
        img_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        img_ci.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        vk_check(vmaCreateImage(allocator, &img_ci, &alloc_ci, &default_image, &default_image_alloc, nullptr));

        // Staging
        VkBufferCreateInfo staging_buffer_info = {};
        staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        staging_buffer_info.size = sizeof(image_data);
        staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VmaAllocationCreateInfo staging_alloc_info = {};
        staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        // Copy staging
        VkBuffer staging_buffer{};
        VmaAllocation staging_allocation{};
        vk_check(vmaCreateBuffer(allocator, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_allocation, nullptr));

        void *data;
        vmaMapMemory(allocator, staging_allocation, &data);
        std::memcpy(data, image_data, sizeof(image_data));
        vmaUnmapMemory(allocator, staging_allocation);

        // Copy to image

        // Allocate cmd buffer
        VkCommandBufferAllocateInfo cmd_alloc_info{};
        cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc_info.commandPool = cmd_pool;
        cmd_alloc_info.commandBufferCount = 1;
        cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer cmd{};
        vk_check(vkAllocateCommandBuffers(device, &cmd_alloc_info, &cmd));

        VkCommandBufferBeginInfo transfer_begin_info{};
        transfer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        transfer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &transfer_begin_info);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = default_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkBufferImageCopy copy_region{};
        copy_region.bufferOffset = 0;
        copy_region.bufferRowLength = 0;
        copy_region.bufferImageHeight = 0;

        copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = 0;
        copy_region.imageSubresource.layerCount = 1;

        copy_region.imageOffset = {0, 0, 0};
        copy_region.imageExtent = {
            2,
            2,
            1};

        vkCmdCopyBufferToImage(cmd, staging_buffer, default_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;

        vkQueueSubmit(queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(queue);

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = default_image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        vk_check(vkCreateImageView(device, &view_info, nullptr, &default_image_view));
        vmaDestroyBuffer(allocator, staging_buffer, staging_allocation);
    }

    void Graphics::destroy_default_image_sampler()
    {
        vkDestroySampler(device, default_sampler, nullptr);
        vkDestroyImageView(device, default_image_view, nullptr);
        vmaDestroyImage(allocator, default_image, default_image_alloc);
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
        cam_proj[1][1] *= -1;

        glm::mat4x4 inv_proj = glm::inverse(cam_proj * global_uniform.view);

        glm::vec3 center{};
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

    const data::Swapchain &Graphics::get_swapchain() const
    {
        return swapchain.value();
    }

    const data::g_buffer::GBuffer &Graphics::get_g_buffer() const
    {
        return *geometry_buffer;
    }

    const data::ShadowPipeline &Graphics::get_shadow_pipeline() const
    {
        return *shadow_pipeline;
    }

    const data::SSAO &Graphics::get_ssao() const
    {
        return *ssao;
    }

    const data::PBRPipeline &Graphics::get_pbr_pipeline() const
    {
        return *pbr_pipeline;
    }

    const data::AmbientLight &Graphics::get_final_ambient() const
    {
        return *final_ambient;
    }

    const data::terrain::TerrainPipeline &Graphics::get_terrain_pipeline() const
    {
        return *terrain_pipeline;
    }

    const data::DirectionalLight &Graphics::get_directional_light() const
    {
        return *directional_light;
    }

    const data::PointLight &Graphics::get_point_light() const
    {
        return *point_light;
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
        directional_light_shadow_map_resize_requested = true;
    }
    void Graphics::set_ssao_bias_and_radius(float bias, float radius)
    {
        ssao_bias = bias;
        ssao_radius = radius;
    }
    VkExtent2D Graphics::get_scaled_draw_extent()
    {
        return VkExtent2D{(unsigned int)std::floor(draw_extent.width * draw_extent_scale),
                          (unsigned int)std::floor(draw_extent.height * draw_extent_scale)};
    }

}
