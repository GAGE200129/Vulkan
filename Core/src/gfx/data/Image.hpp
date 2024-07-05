#pragma once

#include <vk_mem_alloc.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class Image
    {
    public:
        Image(Graphics& gfx, const void* image_data, uint32_t width, uint32_t height);
        Image(const Image&) = delete;
        Image(Image&&) = delete;
        Image operator=(const Image&) = delete;
        ~Image();

        VkImage get_image() const;
        VkImageView get_image_view() const;
        VkSampler get_sampler() const;
    private:
        Graphics& gfx;
        VkFormat image_format{VK_FORMAT_R8G8B8A8_UNORM};
        VkImage image{};
        VkImageView image_view{};
        VmaAllocation allocation{};
        VkSampler sampler{};
    };
}