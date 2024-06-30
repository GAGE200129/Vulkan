#pragma once
#include "IBindable.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <vk_mem_alloc.h>


namespace gage::gfx::bind
{
    class Texture : public IBindable
    {
    public:
        Texture(Graphics& gfx, const utils::Image& in_image);

        void bind(Graphics& gfx) override;
        void destroy(Graphics& gfx) override;

        VkImage get_image() const;
        VkImageView get_image_view() const;
        VkSampler get_sampler() const;
    private:
        VkFormat image_format{VK_FORMAT_R8G8B8A8_UNORM};
        VkImage image{};
        VkImageView image_view{};
        VmaAllocation allocation{};
        VkSampler sampler{};

    };
}