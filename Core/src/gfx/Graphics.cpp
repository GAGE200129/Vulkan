#include <pch.hpp>
#include "Graphics.hpp"

#include <Core/src/log/Log.hpp>
#include <Core/src/utils/VulkanHelper.hpp>


#include "Exception.hpp"

#include "data/Camera.hpp"
#include "data/DefaultPipeline.hpp"


using namespace std::string_literals;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
    void *p_user_data)
{
    using namespace gage;
    gfx::Graphics *gfx = (gfx::Graphics *)p_user_data;
    std::stringstream ss;
    ss << "[Vulkan debug report]: " << p_callback_data->pMessage;
    std::string str = ss.str();
    switch (message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    {
        logger.info(gfx->get_app_name());
        logger.trace(str);
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        logger.info(str);
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        logger.warn(str);
        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        logger.error(str);
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

        vk_check(glfwCreateWindowSurface(instance, window, nullptr, &surface),
                 "Failed to create window surface !");
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
        logger.info("Selected physical device: " + vkb_physical_device.name);

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


        //Create descriptor pool
        VkDescriptorPoolCreateInfo desc_pool_ci{};
        desc_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_ci.maxSets = 4;
        desc_pool_ci.pPoolSizes = pool_sizes;
        desc_pool_ci.poolSizeCount = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
        desc_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        vkCreateDescriptorPool(device, &desc_pool_ci, nullptr, &desc_pool);
        delete_stack.push([this]()
                          { vkDestroyDescriptorPool(device, desc_pool, nullptr); });

        //Define a global set layout
        // GLOBAL SET LAYOUT
        VkDescriptorSetLayoutBinding global_binding{};
        global_binding.binding = 0;
        global_binding.descriptorCount = 1;
        global_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        global_binding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = 1;
        layout_ci.pBindings = &global_binding;
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(device, &layout_ci, nullptr, &global_set_layout));
        delete_stack.push([this]()
        {
            vkDestroyDescriptorSetLayout(device, global_set_layout, nullptr);
        });


        // use vkbootstrap to get a Graphics queue family
        auto queue_family_result = vkb_device.get_queue_index(vkb::QueueType::graphics);
        vkb_check(queue_family_result);
        queue_family = queue_family_result.value();
        //Get queue
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
            VkDescriptorSet& global_set = data.global_set;
            VkBuffer& global_buffer = data.global_buffer;
            VmaAllocation& global_alloc = data.global_alloc;
            VmaAllocationInfo& global_alloc_info = data.global_alloc_info;

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

            //Create global uniform buffer
            VkBufferCreateInfo buffer_ci = {};
            buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_ci.size = sizeof(GlobalUniform);
            buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            VmaAllocationCreateInfo alloc_ci = {};
            alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
            alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            vk_check(vmaCreateBuffer(allocator, &buffer_ci, &alloc_ci, &global_buffer, &global_alloc, &global_alloc_info));

            delete_stack.push([&]()
            {
                vmaDestroyBuffer(allocator, global_buffer, global_alloc);
            });

            // Allocate descriptor set
            VkDescriptorSetAllocateInfo global_set_alloc_info{};
            global_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            global_set_alloc_info.descriptorPool = desc_pool;
            global_set_alloc_info.descriptorSetCount = 1;
            global_set_alloc_info.pSetLayouts = &global_set_layout;
            vkAllocateDescriptorSets(device, &global_set_alloc_info, &global_set);
            delete_stack.push([&, this]()
            {
                vkFreeDescriptorSets(device, desc_pool, 1, &global_set);
            });

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

        default_pipeline = std::make_unique<data::DefaultPipeline>(*this);
        delete_stack.push([this]()
                          { default_pipeline.reset(); });
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

    void Graphics::clear(const data::Camera &camera)
    {
        uploading_mutex.lock();
        

        // Check for swapchain recreation
        if (resize_requested)
        {
            vkDeviceWaitIdle(device);
            draw_extent = draw_extent_temp;
            swapchain.reset();
            swapchain.emplace(*this);
            default_pipeline->reset_pipeline();

            resize_requested = false;
        }


        VkSemaphore &present_semaphore = frame_datas[frame_index].present_semaphore;
        VkFence &render_fence = frame_datas[frame_index].render_fence;
        VmaAllocationInfo& global_alloc_info = frame_datas[frame_index].global_alloc_info;

        //Update global set of currnet frame
        global_uniform.camera_position = camera.get_position();
        global_uniform.projection = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                                                  (float)draw_extent.width, (float)draw_extent.height, camera.get_near(), camera.get_far());
        global_uniform.projection[1][1] *= -1;
        global_uniform.view = camera.get_view();
        std::memcpy(global_alloc_info.pMappedData, &global_uniform, sizeof(GlobalUniform));

        // wait until the GPU has finished rendering the last frame. Timeout of 1 second
        vk_check(vkWaitForFences(device, 1, &render_fence, true, 1000000000));
        vk_check(vkResetFences(device, 1, &render_fence));

        if (VkResult result = vkAcquireNextImageKHR(device, swapchain->get(), 1000000000, present_semaphore, nullptr, &swapchain_image_index);
            result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            logger.fatal("Failed to acquire next image: ").vk_result(result);
            throw GraphicsException{};
        }

    }

    void Graphics::end_frame()
    {

        VkSemaphore &present_semaphore = frame_datas[frame_index].present_semaphore;
        VkSemaphore &render_semaphore = frame_datas[frame_index].render_semaphore;
        VkFence &render_fence = frame_datas[frame_index].render_fence;
        VkCommandBuffer &cmd = frame_datas[frame_index].cmd;
        
        VkImageBlit region{};
        VkImageMemoryBarrier swapchain_memory_barrier{};
        swapchain_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        // reset the command buffer
        vk_check(vkResetCommandBuffer(cmd, 0));
        VkCommandBufferBeginInfo cmd_begin_info = {};
        cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vk_check(vkBeginCommandBuffer(cmd, &cmd_begin_info));


        //Blit swapchain color image to swapchain image and wait for color result
        swapchain_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        swapchain_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        swapchain_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        swapchain_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        swapchain_memory_barrier.image = swapchain->at(swapchain_image_index);
        swapchain_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapchain_memory_barrier.subresourceRange.baseMipLevel = 0;
        swapchain_memory_barrier.subresourceRange.levelCount = 1;
        swapchain_memory_barrier.subresourceRange.baseArrayLayer = 0;
        swapchain_memory_barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &swapchain_memory_barrier
        );

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

        vkCmdBlitImage(cmd, default_pipeline->get_color_image_handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       swapchain->at(swapchain_image_index), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &region, VK_FILTER_LINEAR);

        swapchain_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        swapchain_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        swapchain_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        swapchain_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        swapchain_memory_barrier.image = swapchain->at(swapchain_image_index);
        swapchain_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapchain_memory_barrier.subresourceRange.baseMipLevel = 0;
        swapchain_memory_barrier.subresourceRange.levelCount = 1;
        swapchain_memory_barrier.subresourceRange.baseArrayLayer = 0;
        swapchain_memory_barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &swapchain_memory_barrier
        );
        vk_check(vkEndCommandBuffer(cmd));


        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        std::array<VkSemaphore, 2> wait_semaphores = {
            present_semaphore,
            default_pipeline->get_render_finished_semaphore(this->frame_index)
        };
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
            logger.fatal("Failed to present swapchain image: ").vk_result(result);
            throw GraphicsException{};
        }

        frame_index = (frame_index + 1) % FRAMES_IN_FLIGHT;

        uploading_mutex.unlock();
    }

    Graphics::GlobalUniform& Graphics::get_global_uniform()
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

    const data::Swapchain &Graphics::get_swapchain() const
    {
        return swapchain.value();
    }

    const data::DefaultPipeline &Graphics::get_default_pipeline() const
    {
        return *default_pipeline;
    }

    data::DefaultPipeline &Graphics::get_default_pipeline()
    {
        return *default_pipeline;
    }

    void Graphics::resize(int width, int height)
    {
        draw_extent_temp.width = width;
        draw_extent_temp.height = height;
        resize_requested = true;
    }

    VkExtent2D Graphics::get_scaled_draw_extent()
    {
        return VkExtent2D{(unsigned int)std::floor(draw_extent.width * draw_extent_scale),
                          (unsigned int)std::floor(draw_extent.height * draw_extent_scale)};
    }

}
