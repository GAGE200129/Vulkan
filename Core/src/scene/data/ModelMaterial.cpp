#include <pch.hpp>
#include "ModelMaterial.hpp"
#include "../systems/Renderer.hpp"

#include <Core/src/gfx/data/Image.hpp>

namespace gage::scene::data
{
    ModelMaterial::ModelMaterial(const gfx::Graphics& gfx, const systems::Renderer& renderer, const tinygltf::Model &gltf_model, const tinygltf::Material &gltf_material) :
        gfx(gfx),
        renderer(renderer)
    {
        uniform_buffer_data.color = {
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(0),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(1),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(2),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(3),
        };

        gfx::data::ImageCreateInfo image_ci{};
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.min_filter = VK_FILTER_NEAREST;
        image_ci.mag_filter = VK_FILTER_NEAREST;
        image_ci.address_node = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        image_ci.mip_levels = 1;

        // Has albedo texture ?
        const auto &albedo_texture_index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
        uniform_buffer_data.has_albedo = albedo_texture_index > -1;
        if (uniform_buffer_data.has_albedo)
        {
            const auto &image_src_index = gltf_model.textures.at(albedo_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;

            image_ci.image_data = image.image.data();
            image_ci.width = image.width;
            image_ci.height = image.height;
            image_ci.size_in_bytes = size_in_bytes;
            albedo_image = std::make_unique<gfx::data::Image>(gfx, image_ci);
        }

        // Has metalic roughness ?
        const auto &metalic_roughness_texture_index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        uniform_buffer_data.has_metalic = metalic_roughness_texture_index > -1;
        if (uniform_buffer_data.has_metalic)
        {
            const auto &image_src_index = gltf_model.textures.at(metalic_roughness_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;

            image_ci.image_data = image.image.data();
            image_ci.width = image.width;
            image_ci.height = image.height;
            image_ci.size_in_bytes = size_in_bytes;
            metalic_roughness_image = std::make_unique<gfx::data::Image>(gfx, image_ci);
        }

        // Has normal map ?
        const auto &normal_texture_index = gltf_material.normalTexture.index;
        uniform_buffer_data.has_normal = normal_texture_index > -1;
        if (uniform_buffer_data.has_normal)
        {
            const auto &image_src_index = gltf_model.textures.at(normal_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;

            image_ci.image_data = image.image.data();
            image_ci.width = image.width;
            image_ci.height = image.height;
            image_ci.size_in_bytes = size_in_bytes;
            normal_image = std::make_unique<gfx::data::Image>(gfx, image_ci);
        }

        uniform_buffer = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(data::ModelMaterial::UniformBuffer), &uniform_buffer_data);

        systems::Renderer::MaterialSetAllocInfo alloc_info{
            .size_in_bytes = sizeof(data::ModelMaterial::UniformBuffer),
            .buffer = uniform_buffer->get_buffer_handle(),
            .albedo_view = albedo_image ? albedo_image->get_image_view() : VK_NULL_HANDLE,
            .albedo_sampler = albedo_image ? albedo_image->get_sampler() : VK_NULL_HANDLE,
            .metalic_roughness_view = metalic_roughness_image ? metalic_roughness_image->get_image_view() : VK_NULL_HANDLE,
            .metalic_roughness_sampler = metalic_roughness_image ? metalic_roughness_image->get_sampler() : VK_NULL_HANDLE,
            .normal_view = normal_image ? normal_image->get_image_view() : VK_NULL_HANDLE,
            .normal_sampler = normal_image ? normal_image->get_sampler() : VK_NULL_HANDLE};
        descriptor_set = renderer.allocate_material_set(alloc_info);
    }

    ModelMaterial::~ModelMaterial()
    {
        vkFreeDescriptorSets(gfx.device, gfx.desc_pool, 1, &descriptor_set);
    }
}