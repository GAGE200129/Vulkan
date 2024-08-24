#include <pch.hpp>
#include "GlobalDescriptorSetLayout.hpp"

#include "Device.hpp"
#include "DescriptorPool.hpp"
#include "../Exception.hpp"

namespace gage::gfx::data
{
    GlobalDescriptorSetLayout::GlobalDescriptorSetLayout(const Device& device) :
        device(device)
    {
        // Define a global set layout
        //  GLOBAL SET LAYOUT
        VkDescriptorSetLayoutBinding global_bindings[] = {
            {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL, .pImmutableSamplers = nullptr},
            {.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL, .pImmutableSamplers = nullptr},
        };

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = sizeof(global_bindings) / sizeof(global_bindings[0]);
        layout_ci.pBindings = global_bindings;
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(device.device, &layout_ci, nullptr, &layout));
    }

    GlobalDescriptorSetLayout::~GlobalDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(device.device, layout, nullptr);
    }
}