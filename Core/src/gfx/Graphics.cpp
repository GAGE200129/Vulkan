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
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        draw_extent.width = width;
        draw_extent.height = height;

        vkb::InstanceBuilder builder;
        // make the vulkan instance, with basic debug features
        auto vkb_instance_result = builder.set_app_name(app_name.c_str())
                            .request_validation_layers(true)
                            .set_debug_callback(debugCallback)
                            .set_debug_callback_user_data_pointer(this)
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
		    vkDestroyInstance(instance, nullptr);
        });

        vk_check(glfwCreateWindowSurface(instance, window, nullptr, &surface), 
            "Failed to create window surface !");
        delete_stack.push([this]()
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        });

        // vulkan 1.3 features
        VkPhysicalDeviceVulkan13Features features13 = {};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.dynamicRendering = true;

        // use vkbootstrap to select a gpu.
        vkb::PhysicalDeviceSelector selector{vkb_inst};
        auto physical_device_result = selector
            .set_minimum_version(1, 3)
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
        delete_stack.push([this]()
        {
            vkDestroyDevice(device, nullptr);
        });

        //Create allocator
        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
        
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        //allocatorCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorCreateInfo.physicalDevice = physical_device;
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.instance = instance;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
        vmaCreateAllocator(&allocatorCreateInfo, &allocator);
        delete_stack.push([this]()
        {
            vmaDestroyAllocator(allocator);
        });

        create_swapchain();

        delete_stack.push([this]()
        {
            this->destroy_swapchain();
        });


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
        delete_stack.push([this]()
        {
            vkDestroyCommandPool(device, cmd_pool, nullptr);
        });

        //Allocate command buffer
        VkCommandBufferAllocateInfo cmd_alloc_info = {};
		cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_alloc_info.commandPool = cmd_pool;
		cmd_alloc_info.commandBufferCount = 1;

        vk_check(vkAllocateCommandBuffers(device, &cmd_alloc_info, &cmd));
        delete_stack.push([this]()
        {
            vkFreeCommandBuffers(device, cmd_pool, 1, &cmd);
        });

    
        //Create sync structures
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vk_check(vkCreateFence(device, &fenceCreateInfo, nullptr, &render_fence));
        delete_stack.push([this]()
        {
            vkDestroyFence(device, render_fence, nullptr);
        });

        //for the semaphores we don't need any flags
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.flags = 0;

        vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &present_semaphore));
        vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &render_semaphore));

        delete_stack.push([this]()
        {
            vkDestroySemaphore(device, present_semaphore, nullptr);
            vkDestroySemaphore(device, render_semaphore, nullptr);
        });


        graphics_pipeline = std::make_unique<GraphicsPipeline>(device, swapchain_image_format, swapchain_depth_format, draw_extent, delete_stack);

        
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices = {0, 1, 2};
        //make the array 3 vertices long
        vertices.resize(3);


        //vertex positions
        vertices[0].position = { 1.f, 1.f, 0.0f };
        vertices[1].position = {-1.f, 1.f, 0.0f };
        vertices[2].position = { 0.f,-1.f, 0.0f };


        vertex_buffer = std::make_unique<VertexBuffer>(*this, vertices);
        index_buffer = std::make_unique<IndexBuffer>(*this, indices);

        delete_stack.push([this]()
        {
            vertex_buffer.reset();
            index_buffer.reset();
        });
    }   

    Graphics::~Graphics()
    {
        vkDeviceWaitIdle(device);
        
        //Xoa object nguoc lai
		while(!delete_stack.empty())
        {
            delete_stack.top()(); // call the delete function
            delete_stack.pop(); // pop the stack
        }
    }

    void Graphics::draw_test_triangle()
    {
       

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->get());
        VkDeviceSize offsets{0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertex_buffer->buffer, &offsets);
        vkCmdBindIndexBuffer(cmd, index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, 3, 1, 0, 0, 0);

        vkCmdEndRendering(cmd);
    }

    void Graphics::clear()
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
	    transition_image(cmd, swapchain_images[swapchain_image_index], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,  VK_IMAGE_LAYOUT_GENERAL);
        transition_image(cmd, swapchain_depth_image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        VkClearValue color_clear_value{ 
            VkClearColorValue{ 0.5f, 0.0f, 0.0f, 1.0f }
        };

        VkClearValue depth_clear_value{ 
            { 0.0f, 0u }
        };

        VkRenderingAttachmentInfo color_attachment = {};
        color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        color_attachment.clearValue = color_clear_value;
        color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment.imageView = swapchain_image_views[swapchain_image_index]; // Link to current swap chain image view
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingAttachmentInfo depth_attachment = {};
        depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depth_attachment.clearValue = depth_clear_value;
        depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_attachment.imageView = swapchain_depth_image_view; // Link to current swap chain image view
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo render_info = {};
        render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        render_info.renderArea.offset = {0, 0};
        render_info.renderArea.extent = draw_extent;
        render_info.colorAttachmentCount = 1;
        render_info.pColorAttachments = &color_attachment;
        render_info.pDepthAttachment = &depth_attachment;
        render_info.layerCount = 1;
        vkCmdBeginRendering(cmd, &render_info);
        
    }

    void Graphics::end_frame()
    {
        transition_image(cmd, swapchain_images[swapchain_image_index], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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


    void Graphics::create_swapchain()
    {
        vkb::SwapchainBuilder swapchainBuilder{physical_device, device, surface};
        auto swapchain_result = swapchainBuilder
            //.use_default_format_selection()
            .set_desired_format(VkSurfaceFormatKHR { 
                .format = swapchain_image_format,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            })
            //use vsync present mode
            .set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
            .set_desired_extent(draw_extent.width, draw_extent.height)
            .set_desired_min_image_count(vkb::SwapchainBuilder::BufferMode::SINGLE_BUFFERING)
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

        //Create depth image and view
        VkImageCreateInfo depth_image_ci = {};
        depth_image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depth_image_ci.imageType = VK_IMAGE_TYPE_2D;
        depth_image_ci.extent.width = draw_extent.width;
        depth_image_ci.extent.height = draw_extent.height;
        depth_image_ci.extent.depth = 1;
        depth_image_ci.mipLevels = 1;
        depth_image_ci.arrayLayers = 1;
        depth_image_ci.format = swapchain_depth_format;
        depth_image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        depth_image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depth_image_ci.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo depth_image_alloc_info = {};
        depth_image_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
        depth_image_alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        //vk_check(vkCreateImage(device, &depth_image_ci, nullptr, &swapchain_depth_image));
        vk_check(vmaCreateImage(allocator, &depth_image_ci, &depth_image_alloc_info, &swapchain_depth_image, &swapchain_depth_image_allocation, nullptr));
        
        VkImageViewCreateInfo depth_image_view_ci = {};
        depth_image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depth_image_view_ci.image = swapchain_depth_image;
        depth_image_view_ci.format = swapchain_depth_format;
        depth_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depth_image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        depth_image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        depth_image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        depth_image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        depth_image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth_image_view_ci.subresourceRange.baseMipLevel = 0;
        depth_image_view_ci.subresourceRange.baseArrayLayer = 0;
        depth_image_view_ci.subresourceRange.layerCount = 1;
        depth_image_view_ci.subresourceRange.levelCount = 1;

        vk_check(vkCreateImageView(device, &depth_image_view_ci, nullptr, &swapchain_depth_image_view));

        delete_stack.push([this]()
        {
            vmaDestroyImage(allocator, swapchain_depth_image, swapchain_depth_image_allocation);
            vkDestroyImageView(device, swapchain_depth_image_view, nullptr);
        });
        
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
