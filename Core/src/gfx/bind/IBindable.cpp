#include "IBindable.hpp"

namespace gage::gfx::bind
{
    VkDevice IBindable::get_device(Graphics& gfx)
    {
        return gfx.device;
    } 
    VmaAllocator IBindable::get_allocator(Graphics& gfx)
    {
        return gfx.allocator;
    }

    VkCommandBuffer IBindable::get_cmd(Graphics& gfx)
    {
        return gfx.cmd;
    }

    VkExtent2D IBindable::get_draw_extent(Graphics& gfx)
    {
        return gfx.draw_extent;
    }

    VkFormat IBindable::get_swapchain_image_format(Graphics& gfx)
    {
        return gfx.swapchain_image_format;
    }
    VkFormat IBindable::get_swapchain_depth_format(Graphics& gfx)
    {
        return gfx.swapchain_depth_format;
    }

    IBindable::~IBindable()
    {

    }
}