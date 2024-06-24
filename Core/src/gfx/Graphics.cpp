#include "Graphics.hpp"

#include <GLFW/glfw3.h>
#include <string>
#include <sstream>
#include <VkBootstrap.h>
#include <Core/src/log/Log.hpp>

#include "Exception.hpp"
#include "Image.hpp"

using namespace std::string_literals;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
    void *p_user_data)
{
    using namespace gage;
    gfx::Graphics* gfx = (gfx::Graphics*)p_user_data;
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
    Graphics::Graphics(GLFWwindow *window, std::string app_name) :
        app_name{ std::move(app_name) }
    {
        vkb::InstanceBuilder builder;

        // make the vulkan instance, with basic debug features
        auto vkb_instance_result = builder.set_app_name(app_name.c_str())
                            .request_validation_layers(true)
                            .set_debug_callback(debugCallback)
                            .set_debug_callback_user_data_pointer(this)
                            .require_api_version(1, 0, 0)
                            .build();

        vkb_check(vkb_instance_result, "Failed to create vulkan instance !"s);

        vkb::Instance vkb_inst = vkb_instance_result.value();

        // grab the instance
        instance = vkb_inst.instance;
        debug_messenger = vkb_inst.debug_messenger;

        vk_check(glfwCreateWindowSurface(instance, window, nullptr, &surface), 
            "Failed to create window surface !");


        // vulkan 1.3 features
        VkPhysicalDeviceVulkan13Features features13 = {};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.dynamicRendering = true;
        features13.synchronization2 = true;

        // vulkan 1.2 features
        VkPhysicalDeviceVulkan12Features features12 = {};
        features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        // use vkbootstrap to select a gpu.
        vkb::PhysicalDeviceSelector selector{vkb_inst};
        auto physical_device_result = selector
                                                 .set_minimum_version(1, 3)
                                                 .set_required_features_12(features12)
                                                 .set_required_features_13(features13)
                                                 .set_surface(surface)
                                                 .select();
        vkb_check(physical_device_result, "Failed to select physical device: ");
 
        vkb::PhysicalDevice vkb_physical_device = physical_device_result.value();

        vkb::DeviceBuilder deviceBuilder{vkb_physical_device};
        auto vkb_device_result = deviceBuilder.build();
        vkb_check(vkb_device_result, "Failed to create device: ");

        auto vkb_device = vkb_device_result.value();

        device = vkb_device.device;
        physical_device = vkb_physical_device.physical_device;

        create_swapchain(window);

        // use vkbootstrap to get a Graphics queue
        auto graphics_queue_result = vkb_device.get_queue(vkb::QueueType::graphics);
        auto graphics_queue_family_result =  vkb_device.get_queue_index(vkb::QueueType::graphics);
        vkb_check(vkb_device_result);
        vkb_check(graphics_queue_family_result);

        graphics_queue = graphics_queue_result.value();
        graphics_queue_family = graphics_queue_family_result.value();

        //Create command pool
        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_info.queueFamilyIndex = graphics_queue_family;

        vk_check(vkCreateCommandPool(device, &command_pool_info, nullptr, &cmd_pool));

        //Allocate command buffer
        VkCommandBufferAllocateInfo cmd_alloc_info = {};
		cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_alloc_info.commandPool = cmd_pool;
		cmd_alloc_info.commandBufferCount = 1;
		cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        vk_check(vkAllocateCommandBuffers(device, &cmd_alloc_info, &cmd));


        //Create framebuffers
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
    

        //Create sync structures
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vk_check(vkCreateFence(device, &fenceCreateInfo, nullptr, &render_fence));

        //for the semaphores we don't need any flags
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.flags = 0;

        vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &present_semaphore));
        vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &render_semaphore));

    }   

    Graphics::~Graphics()
    {
        vkDeviceWaitIdle(device);

        vkDestroySemaphore(device, present_semaphore, nullptr);
        vkDestroySemaphore(device, render_semaphore, nullptr);
        vkDestroyFence(device, render_fence, nullptr);


        vkDestroyCommandPool(device, cmd_pool, nullptr);
        destroy_swapchain();
        vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);
		
		vkb::destroy_debug_utils_messenger(instance, debug_messenger);
		vkDestroyInstance(instance, nullptr);
    }

    void Graphics::draw_test_triangle()
    {
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    void Graphics::clear(float r, float g, float b)
    {
         //wait until the GPU has finished rendering the last frame. Timeout of 1 second
	    vk_check(vkWaitForFences(device, 1, &render_fence, true, 1000000000));
	    vk_check(vkResetFences(device, 1, &render_fence));
	    vk_check(vkAcquireNextImageKHR(device, swapchain, 1000000000, present_semaphore, nullptr, &swapchain_image_index));

        //reset the command buffer
	    vk_check(vkResetCommandBuffer(cmd, 0));
          //begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
        VkCommandBufferBeginInfo cmd_begin_info = {};
        cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vk_check(vkBeginCommandBuffer(cmd, &cmd_begin_info));

        //make the swapchain image into writeable mode before rendering
	    transition_image(cmd, swapchain_images[swapchain_image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        //make a clear-color from frame number. This will flash with a 120*pi frame period.
        VkClearColorValue clear_value;
        clear_value = { { r, g, b, 1.0f } };

        VkImageSubresourceRange clear_range = {};
        clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clear_range.baseMipLevel = 0;
        clear_range.levelCount = VK_REMAINING_MIP_LEVELS;
        clear_range.baseArrayLayer = 0;
        clear_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

        //clear image
	    vkCmdClearColorImage(cmd, swapchain_images[swapchain_image_index], VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_range);

        //make the swapchain image into presentable mode
	    transition_image(cmd, swapchain_images[swapchain_image_index],VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

       
    }

    void Graphics::end_frame()
    {
        
        vk_check(vkEndCommandBuffer(cmd));

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        submit.pWaitDstStageMask = &waitStage;

        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &present_semaphore;

        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &render_semaphore;

        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        //submit command buffer to the queue and execute it.
        // _renderFence will now block until the graphic commands finish execution
        vk_check(vkQueueSubmit(graphics_queue, 1, &submit, render_fence));
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.pSwapchains = &swapchain;
        presentInfo.swapchainCount = 1;

        presentInfo.pWaitSemaphores = &render_semaphore;
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &swapchain_image_index;

        vk_check(vkQueuePresentKHR(graphics_queue, &presentInfo));
    }


    const std::string& Graphics::get_app_name() const noexcept 
    {
        return app_name;
    }


    void Graphics::create_swapchain(GLFWwindow *window)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        vkb::SwapchainBuilder swapchainBuilder{physical_device, device, surface};
        auto swapchain_result = swapchainBuilder
            //.use_default_format_selection()
            .set_desired_format(VkSurfaceFormatKHR { 
                .format = swapchain_image_format,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            })
            //use vsync present mode
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .set_desired_min_image_count(vkb::SwapchainBuilder::BufferMode::DOUBLE_BUFFERING)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build();
        vkb_check(swapchain_result, "Failed to create swapchain: ");

        auto vkb_swapchain = swapchain_result.value();

        swapchain = vkb_swapchain.swapchain;
        

        auto swapchain_images_result = vkb_swapchain.get_images();
	    auto swapchain_image_views_result = vkb_swapchain.get_image_views();
        vkb_check(swapchain_images_result, "Failed to allocate swapchain images: ");
        vkb_check(swapchain_image_views_result, "Failed to allocate swapchain image views: ");

        swapchain_images = std::move(swapchain_images_result.value());
        swapchain_image_views = std::move(swapchain_image_views_result.value());
    }

    void Graphics::destroy_swapchain()
    {
        vkDestroySwapchainKHR(device, swapchain, nullptr);

        // destroy swapchain resources
        for (size_t i = 0; i < swapchain_image_views.size(); i++) {

            vkDestroyImageView(device, swapchain_image_views[i], nullptr);
        }
    }
}
