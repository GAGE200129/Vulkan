#include "Swapchain.hpp"

#include <Core/src/utils/VulkanHelper.hpp>

#include "../Graphics.hpp"

#include <VkBootstrap.h>
#include <tuple>

namespace gage::gfx::data
{
    Swapchain::Swapchain(Graphics &gfx) : gfx(gfx)
    {
        vkb::SwapchainBuilder swapchainBuilder{gfx.physical_device, gfx.device, gfx.surface};
        auto result = swapchainBuilder
                          //.use_default_format_selection()
                          .set_desired_format(VkSurfaceFormatKHR{
                              .format = image_format,
                              .colorSpace = image_color_space})
                          // use vsync present mode
                          .set_desired_present_mode(present_mode)
                          .set_desired_extent(gfx.draw_extent.width, gfx.draw_extent.height)
                          .set_desired_min_image_count(vkb::SwapchainBuilder::BufferMode::SINGLE_BUFFERING)
                          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                          .build();
        vkb_check(result, "Failed to create swapchain: ");

        auto vkb_swapchain = result.value();

        swapchain = vkb_swapchain.swapchain;

        auto images_result = vkb_swapchain.get_images();
        auto image_views_result = vkb_swapchain.get_image_views();
        vkb_check(images_result, "Failed to allocate swapchain images: ");
        vkb_check(image_views_result, "Failed to allocate swapchain image views: ");

        images = std::move(images_result.value());
        image_views = std::move(image_views_result.value());

        VkExternalMemoryImageCreateInfo external_image{};
        external_image.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
        external_image.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        VkExportMemoryAllocateInfo export_memory{};
        export_memory.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
        export_memory.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        // Create depth image and view
        {
            VkImageCreateInfo depth_image_ci = {};
            depth_image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            depth_image_ci.imageType = VK_IMAGE_TYPE_2D;
            depth_image_ci.extent.width = gfx.get_scaled_draw_extent().width;
            depth_image_ci.extent.height = gfx.get_scaled_draw_extent().height;
            depth_image_ci.extent.depth = 1;
            depth_image_ci.mipLevels = 1;
            depth_image_ci.arrayLayers = 1;
            depth_image_ci.format = depth_format;
            depth_image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
            depth_image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depth_image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            depth_image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
            depth_image_ci.pNext = &external_image;

            vk_check(vkCreateImage(gfx.device, &depth_image_ci, nullptr, &depth_image));

            VkMemoryRequirements mem_reqs{};
            vkGetImageMemoryRequirements(gfx.device, depth_image, &mem_reqs);
            depth_image_memory_size = mem_reqs.size;

            VkMemoryAllocateInfo mem_alloc_info{};
            mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc_info.allocationSize = mem_reqs.size;
            mem_alloc_info.memoryTypeIndex = utils::find_memory_type(gfx.physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            mem_alloc_info.pNext = &export_memory;

            vk_check(vkAllocateMemory(gfx.device, &mem_alloc_info, nullptr, &depth_image_memory));
            vk_check(vkBindImageMemory(gfx.device, depth_image, depth_image_memory, 0));

            VkImageViewCreateInfo depth_image_view_ci = {};
            depth_image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            depth_image_view_ci.image = depth_image;
            depth_image_view_ci.format = depth_format;
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

            vk_check(vkCreateImageView(gfx.device, &depth_image_view_ci, nullptr, &depth_image_view));
        }
        // Create color image and view
        {
            VkImageCreateInfo color_image_ci = {};
            color_image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            color_image_ci.imageType = VK_IMAGE_TYPE_2D;
            color_image_ci.extent.width = gfx.get_scaled_draw_extent().width;
            color_image_ci.extent.height = gfx.get_scaled_draw_extent().height;
            color_image_ci.extent.depth = 1;
            color_image_ci.mipLevels = 1;
            color_image_ci.arrayLayers = 1;
            color_image_ci.format = image_format;
            color_image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
            color_image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            color_image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
            color_image_ci.pNext = &external_image;

            vk_check(vkCreateImage(gfx.device, &color_image_ci, nullptr, &color_image));

            VkMemoryRequirements mem_reqs{};
            vkGetImageMemoryRequirements(gfx.device, color_image, &mem_reqs);
            color_image_memory_size = mem_reqs.size;

            VkMemoryAllocateInfo mem_alloc_info{};
            mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc_info.allocationSize = mem_reqs.size;
            mem_alloc_info.memoryTypeIndex = utils::find_memory_type(gfx.physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            mem_alloc_info.pNext = &export_memory;

            vk_check(vkAllocateMemory(gfx.device, &mem_alloc_info, nullptr, &color_image_memory));
            vk_check(vkBindImageMemory(gfx.device, color_image, color_image_memory, 0));

            VkImageViewCreateInfo color_image_view_ci = {};
            color_image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            color_image_view_ci.image = color_image;
            color_image_view_ci.format = image_format;
            color_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            color_image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            color_image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            color_image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            color_image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            color_image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            color_image_view_ci.subresourceRange.baseMipLevel = 0;
            color_image_view_ci.subresourceRange.baseArrayLayer = 0;
            color_image_view_ci.subresourceRange.layerCount = 1;
            color_image_view_ci.subresourceRange.levelCount = 1;

            vk_check(vkCreateImageView(gfx.device, &color_image_view_ci, nullptr, &color_image_view));
        }
    }
    Swapchain::~Swapchain()
    {
        vkDestroySwapchainKHR(gfx.device, swapchain, nullptr);

        // destroy swapchain resources
        for (size_t i = 0; i < image_views.size(); i++)
        {
            vkDestroyImageView(gfx.device, image_views[i], nullptr);
        }

        vkDestroyImage(gfx.device, depth_image, nullptr);
        vkFreeMemory(gfx.device, depth_image_memory, nullptr);
        vkDestroyImageView(gfx.device, depth_image_view, nullptr);

        vkDestroyImage(gfx.device, color_image, nullptr);
        vkFreeMemory(gfx.device, color_image_memory, nullptr);
        vkDestroyImageView(gfx.device, color_image_view, nullptr);
    }

    std::tuple<uint32_t, uint32_t> Swapchain::get_color_image_external() const
    {
        VkMemoryGetFdInfoKHR get_handle_info{};
        get_handle_info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
        get_handle_info.memory = color_image_memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        int fd;
        auto vkGetMemoryFdKHR = PFN_vkGetMemoryFdKHR(vkGetDeviceProcAddr(gfx.device, "vkGetMemoryFdKHR"));
        vkGetMemoryFdKHR(gfx.device, &get_handle_info, &fd);
        return std::make_tuple<uint32_t, uint32_t>(fd, color_image_memory_size);
    }
    std::tuple<uint32_t, uint32_t> Swapchain::get_depth_image_external() const
    {
        VkMemoryGetFdInfoKHR get_handle_info{};
        get_handle_info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
        get_handle_info.memory = depth_image_memory;
        get_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        int fd;
        auto vkGetMemoryFdKHR = PFN_vkGetMemoryFdKHR(vkGetDeviceProcAddr(gfx.device, "vkGetMemoryFdKHR"));
        vkGetMemoryFdKHR(gfx.device, &get_handle_info, &fd);
        return std::make_tuple<uint32_t, uint32_t>(fd, depth_image_memory_size);
    }

    VkSwapchainKHR Swapchain::get() const
    {
        return swapchain;
    }

    VkImage Swapchain::get_color_image_handle() const
    {
        return color_image;
    }
    VkImage Swapchain::get_depth_image_handle() const
    {
        return depth_image;
    }

    VkImageView Swapchain::get_color_image_view() const
    {
        return color_image_view;
    }
    VkImageView Swapchain::get_depth_image_view() const
    {
        return depth_image_view;
    }

    VkFormat Swapchain::get_image_format() const
    {
        return image_format;
    }
    VkFormat Swapchain::get_depth_format() const
    {
        return depth_format;
    }

    VkImage Swapchain::at(size_t i) const
    {
        assert(i < images.size());
        return images.at(i);
    }
}