#pragma once
#include "IBindable.hpp"


#include <vk_mem_alloc.h>

namespace gage::gfx::bind
{
    class Texture;
    class DescriptorSet : public IBindable
    {
    public:
        DescriptorSet(Graphics& gfx, VkPipelineLayout pipeline_layout, VkDescriptorSetLayout layouts);

        void set_texture(Graphics& gfx, uint32_t binding, const Texture& texture);
        void set_buffer(Graphics& gfx, uint32_t binding, VkBuffer buffer, uint32_t size, VkDescriptorType type);

        void bind(Graphics& gfx) override;
        void destroy(Graphics& gfx) override;
    private:
        VkPipelineLayout pipeline_layout{};
        VkDescriptorSet descriptor_set{};
    };
}