#include <pch.hpp>
#include "Swapchain.hpp"

#include <Core/src/utils/VulkanHelper.hpp>

#include "Device.hpp"
#include "Instance.hpp"

#include "../Exception.hpp"

namespace gage::gfx::data
{
    Swapchain::Swapchain(const Instance& instance, const Device& device, VkExtent2D draw_extent) :
        instance(instance),
        device(device),
        draw_extent(draw_extent)
    {
        vkb::SwapchainBuilder swapchainBuilder{device.physical_device, device.device, instance.surface};
        auto result = swapchainBuilder
                          //.use_default_format_selection()
                          .set_desired_format(VkSurfaceFormatKHR{
                              .format = COLOR_FORMAT,
                              .colorSpace = COLOR_SPACE})
                          // use vsync present mode
                          .set_desired_present_mode(PRESENT_MODE)
                          .set_desired_extent(draw_extent.width, draw_extent.height)
                          .set_desired_min_image_count(vkb::SwapchainBuilder::BufferMode::DOUBLE_BUFFERING)
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
    }
    Swapchain::~Swapchain()
    {
        vkDestroySwapchainKHR(device.device, swapchain, nullptr);

        // destroy swapchain resources
        for (size_t i = 0; i < image_views.size(); i++)
        {
            vkDestroyImageView(device.device, image_views[i], nullptr);
        }
    }


    void Swapchain::reset()
    {
        images.clear();
        image_views.clear();
        
        vkDestroySwapchainKHR(device.device, swapchain, nullptr);

        // destroy swapchain resources
        for (size_t i = 0; i < image_views.size(); i++)
        {
            vkDestroyImageView(device.device, image_views[i], nullptr);
        }

        vkb::SwapchainBuilder swapchainBuilder{device.physical_device, device.device, instance.surface};
        auto result = swapchainBuilder
                          //.use_default_format_selection()
                          .set_desired_format(VkSurfaceFormatKHR{
                              .format = COLOR_FORMAT,
                              .colorSpace = COLOR_SPACE})
                          // use vsync present mode
                          .set_desired_present_mode(PRESENT_MODE)
                          .set_desired_extent(draw_extent.width, draw_extent.height)
                          .set_desired_min_image_count(vkb::SwapchainBuilder::BufferMode::DOUBLE_BUFFERING)
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

    }

   

    VkSwapchainKHR Swapchain::get() const
    {
        return swapchain;
    }

    VkImage Swapchain::at(size_t i) const
    {
        assert(i < images.size());
        return images.at(i);
    }
}