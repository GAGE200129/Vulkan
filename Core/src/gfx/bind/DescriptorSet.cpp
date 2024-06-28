#include "DescriptorSet.hpp"

#include <cstring>

namespace gage::gfx::bind
{
    DescriptorSet::DescriptorSet(Graphics &gfx, VkPipelineLayout pipeline_layout, VkDescriptorSetLayout layout) : pipeline_layout(pipeline_layout)
    {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorSetCount = 1;
        alloc_info.descriptorPool = get_desc_pool(gfx);
        alloc_info.pSetLayouts = &layout;
        vk_check(vkAllocateDescriptorSets(get_device(gfx), &alloc_info, &descriptor_set));
    }

    void DescriptorSet::add_combined_image_sampler(Graphics &gfx, uint32_t binding, VkImageView image, VkSampler sampler)
    {
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = image;
        image_info.sampler = sampler;

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_set;
        descriptor_write.dstBinding = binding;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &image_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(get_device(gfx), 1, &descriptor_write, 0, nullptr);
    }

    void DescriptorSet::add_buffer(Graphics &gfx, uint32_t binding, VkBuffer buffer, uint32_t size, VkDescriptorType type)
    {
        // Bind uniform buffer to binding 0 of uniform buffer
        VkDescriptorBufferInfo buffer_desc_info{};
        buffer_desc_info.buffer = buffer;
        buffer_desc_info.offset = 0;
        buffer_desc_info.range = sizeof(size);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_set;
        descriptor_write.dstBinding = binding;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = type;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_desc_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(get_device(gfx), 1, &descriptor_write, 0, nullptr);
    }

    void DescriptorSet::bind(Graphics &gfx)
    {
        vkCmdBindDescriptorSets(get_cmd(gfx), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    }
    void DescriptorSet::destroy(Graphics &gfx)
    {
        vk_check(vkFreeDescriptorSets(get_device(gfx), get_desc_pool(gfx), 1, &descriptor_set));
    }
}