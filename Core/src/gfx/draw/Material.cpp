#include "Material.hpp"

#include "../Graphics.hpp"

#include <tiny_gltf.h>

namespace gage::gfx::draw
{
    Material::Material(Graphics &gfx, const tinygltf::Model &model, const tinygltf::Material &gltf_material) : gfx(gfx)
    {
        uniform_buffer_data.color = {
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(0),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(1),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(2),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(3),
        };

        

        //Has albedo texture ?
        uniform_buffer_data.has_albedo = gltf_material.pbrMetallicRoughness.baseColorTexture.index > -1;
        if (uniform_buffer_data.has_albedo)
        {
            const auto &image_src_index = model.textures.at(gltf_material.pbrMetallicRoughness.baseColorTexture.index).source;
            const auto &image = model.images.at(image_src_index);
            diffuse_image.emplace(gfx, image.image.data(), image.width, image.height);
        }

       
        uniform_buffer.emplace(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBuffer), &uniform_buffer_data);
        descriptor_set = gfx.get_default_pipeline().allocate_instance_set(
            sizeof(UniformBuffer), uniform_buffer.value().get_buffer_handle(),
            diffuse_image.has_value() ? diffuse_image.value().get_image_view() : VK_NULL_HANDLE, 
            diffuse_image.has_value() ? diffuse_image.value().get_sampler() : VK_NULL_HANDLE
        );

        
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