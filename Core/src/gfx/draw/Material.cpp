#include "Material.hpp"

#include "../Graphics.hpp"

#include <tiny_gltf.h>

namespace gage::gfx::draw
{
    Material::Material(Graphics& gfx, const tinygltf::Material& gltf_material) :
        gfx(gfx),
        uniform_buffer(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBuffer), &uniform_buffer_data)
    {
        uniform_buffer_data.color = {
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(0),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(1),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(2),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(3),
        };
        descriptor_set = gfx.get_default_pipeline().allocate_instance_set(sizeof(UniformBuffer), uniform_buffer.get_buffer_handle());
        std::memcpy(uniform_buffer.get_mapped(), &uniform_buffer_data, sizeof(UniformBuffer));
    }


    Material::~Material()
    {
        gfx.get_default_pipeline().free_instance_set(descriptor_set);
    }

    VkDescriptorSet Material::get_desc_set() const
    {
        return descriptor_set;
    }
}