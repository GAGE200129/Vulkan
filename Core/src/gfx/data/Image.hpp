#pragma once


namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    struct ImageCreateInfo
    {
        VkFormat format;
        VkFilter min_filter;
        VkFilter mag_filter;
        VkSamplerAddressMode address_node;
    };

    class Image
    {
    public:
        Image(Graphics& gfx, const void* image_data, uint32_t width, uint32_t height, size_t size_in_bytes, ImageCreateInfo create_info);
        Image(const Image&) = delete;
        Image(Image&&) = delete;
        Image operator=(const Image&) = delete;
        ~Image();

        VkImage get_image() const;
        VkImageView get_image_view() const;
        VkSampler get_sampler() const;
    private:
        Graphics& gfx;
        VkImage image{};
        VkImageView image_view{};
        VmaAllocation allocation{};
        VkSampler sampler{};
    };
}