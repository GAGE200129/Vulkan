#pragma once
#include "../Graphics.hpp"

namespace gage::gfx::bind
{
    class IBindable
    {
    public:
        virtual void bind(Graphics& gfx) = 0;
        virtual void destroy(Graphics& gfx) = 0;
        virtual ~IBindable();

    protected:
        static VkDevice get_device(Graphics& gfx); 
        static VmaAllocator get_allocator(Graphics& gfx); 
        static VkCommandBuffer get_cmd(Graphics& gfx); 
        static VkCommandBuffer get_transfer_cmd(Graphics& gfx); 
        static VkQueue get_queue(Graphics& gfx);
        static VkExtent2D get_draw_extent(Graphics& gfx);
        static VkDescriptorPool get_desc_pool(Graphics& gfx);
        static VkFormat get_swapchain_image_format(Graphics& gfx);
        static VkFormat get_swapchain_depth_format(Graphics& gfx);
    };
};