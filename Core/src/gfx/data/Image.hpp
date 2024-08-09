#pragma once

#include <vk_mem_alloc.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    struct ImageCreateInfo
    {
        const void* image_data{};
        uint32_t width{}, height{};
        uint32_t mip_levels{};
        uint32_t size_in_bytes{};
        VkFormat format{};
        VkFilter min_filter{};
        VkFilter mag_filter{};
        VkSamplerAddressMode address_node{};
    };

    class Image
    {
    public:
        Image(const Graphics& gfx, ImageCreateInfo ci);
        Image(Image&&) = default;
        Image(const Image&) = delete;
        Image operator=(const Image&) = delete;
        ~Image();

        VkImage get_image() const;
        VkImageView get_image_view() const;
        VkSampler get_sampler() const;
    private:
        void generate_mip_maps(VkCommandBuffer cmd, uint32_t mip_levels, uint32_t width, uint32_t height);
    private:
        const Graphics& gfx;
        VkImage image{};
        VkImageView image_view{};
        VmaAllocation allocation{};
        VkSampler sampler{};
    };
}