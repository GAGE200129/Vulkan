#include "IBindable.hpp"

#include "../data/Swapchain.hpp"

namespace gage::gfx::bind
{
    VkDevice IBindable::get_device(Graphics &gfx)
    {
        return gfx.device;
    }
    VmaAllocator IBindable::get_allocator(Graphics &gfx)
    {
        return gfx.allocator;
    }

    VkCommandBuffer IBindable::get_cmd(Graphics &gfx)
    {
        return gfx.frame_datas[gfx.frame_index].cmd;
    }

    VkCommandBuffer IBindable::get_transfer_cmd(Graphics &gfx)
    {
        return gfx.transfer_cmd;
    }

    VkQueue IBindable::get_queue(Graphics& gfx)
    {
        return gfx.graphics_queue;
    }

    VkExtent2D IBindable::get_draw_extent(Graphics &gfx)
    {
        return gfx.get_scaled_draw_extent();
    }

    VkFormat IBindable::get_swapchain_image_format(Graphics &gfx)
    {
        return gfx.get_swapchain().get_image_format();
    }
    VkFormat IBindable::get_swapchain_depth_format(Graphics &gfx)
    {
        return gfx.get_swapchain().get_depth_format();
    }

    VkDescriptorPool IBindable::get_desc_pool(Graphics& gfx)
    {
        return gfx.desc_pool;
    }

    IBindable::~IBindable()
    {
    }
}