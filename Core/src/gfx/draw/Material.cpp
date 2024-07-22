#include "pch.hpp"
#include "Material.hpp"

#include "../Graphics.hpp"
#include "../data/PBRPipeline.hpp"



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

        data::ImageCreateInfo image_ci{VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT};

        //Has albedo texture ?
        const auto& albedo_texture_index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
        uniform_buffer_data.has_albedo = albedo_texture_index > -1;
        if (uniform_buffer_data.has_albedo)
        {
            const auto &image_src_index = model.textures.at(albedo_texture_index).source;
            const auto &image = model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;
            albedo_image.emplace(gfx, image.image.data(), image.width, image.height, size_in_bytes, image_ci);
        }

        //Has metalic roughness ?
        const auto& metalic_roughness_texture_index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        uniform_buffer_data.has_metalic = metalic_roughness_texture_index > -1;
        if (uniform_buffer_data.has_metalic)
        {
            const auto &image_src_index = model.textures.at(metalic_roughness_texture_index).source;
            const auto &image = model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;
            metalic_roughness_image.emplace(gfx, image.image.data(), image.width, image.height, size_in_bytes, image_ci);
        }

        //Has normal map ?
        const auto& normal_texture_index = gltf_material.normalTexture.index;
        uniform_buffer_data.has_normal = normal_texture_index > -1;
        if (uniform_buffer_data.has_normal)
        {
            const auto &image_src_index = model.textures.at(normal_texture_index).source;
            const auto &image = model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;
            normal_image.emplace(gfx, image.image.data(), image.width, image.height, size_in_bytes,  image_ci);
        }

       
        uniform_buffer.emplace(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBuffer), &uniform_buffer_data);
        descriptor_set = gfx.get_pbr_pipeline().allocate_material_set(
            sizeof(UniformBuffer), uniform_buffer.value().get_buffer_handle(),
            albedo_image.has_value() ? albedo_image.value().get_image_view() : VK_NULL_HANDLE, 
            albedo_image.has_value() ? albedo_image.value().get_sampler() : VK_NULL_HANDLE,
            metalic_roughness_image.has_value() ? metalic_roughness_image.value().get_image_view() : VK_NULL_HANDLE,
            metalic_roughness_image.has_value() ? metalic_roughness_image.value().get_sampler() : VK_NULL_HANDLE,
            normal_image.has_value() ? normal_image.value().get_image_view() : VK_NULL_HANDLE,
            normal_image.has_value() ? normal_image.value().get_sampler() : VK_NULL_HANDLE
        );

        
    }

    Material::~Material()
    {
        gfx.get_pbr_pipeline().free_descriptor_set(descriptor_set);
    }

    VkDescriptorSet Material::get_desc_set() const
    {
        return descriptor_set;
    }
}