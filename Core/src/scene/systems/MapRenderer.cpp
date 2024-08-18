#include <pch.hpp>
#include "MapRenderer.hpp"

#include "../Node.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/g_buffer/GBuffer.hpp>
#include <Core/src/gfx/data/GPUBuffer.hpp>
#include <Core/src/gfx/data/Image.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace gage::scene::systems
{
    MapRenderer::MapRenderer(const gfx::Graphics &gfx) : gfx(gfx)
    {
        create_pipeline();
        create_depth_pipeline();
    }
    MapRenderer::~MapRenderer()
    {
        vkDestroyPipelineLayout(gfx.device, depth_pipeline_layout, nullptr);
        vkDestroyPipeline(gfx.device, depth_pipeline, nullptr);

        vkDestroyDescriptorSetLayout(gfx.device, desc_layout, nullptr);
        vkDestroyPipelineLayout(gfx.device, pipeline_layout, nullptr);
        vkDestroyPipeline(gfx.device, pipeline, nullptr);
    }

    void MapRenderer::init()
    {
        auto load_image = [this](const std::string &file_path, uint32_t &out_width, uint32_t &out_height) -> std::unique_ptr<gfx::data::Image>
        {
            int w, h, comp;
            stbi_uc *data = stbi_load(file_path.c_str(), &w, &h, &comp, STBI_rgb);

            unsigned char error_image_data[] =
                {
                    0, 255, 0, 255, 0, 0,
                    255, 255, 0, 255, 0, 255};

            if (!data)
            {
                w = 2;
                h = 2;
                data = error_image_data;
            }

            gfx::data::ImageCreateInfo ci{};
            ci.address_node = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            ci.format = VK_FORMAT_R8G8B8_UNORM;
            ci.mip_levels = 1;
            ci.width = w;
            ci.height = h;
            ci.min_filter = VK_FILTER_NEAREST;
            ci.mag_filter = VK_FILTER_NEAREST;
            ci.image_data = data;
            ci.size_in_bytes = w * h * 3;
            auto image = std::make_unique<gfx::data::Image>(gfx, ci);

            if (data != error_image_data)
            {
                stbi_image_free(data);
            }

            out_width = w;
            out_height = h;

            return image;
        };

        auto create_descriptor_set = [this](VkImageView image_view, VkSampler sampler) -> VkDescriptorSet
        {
            VkDescriptorSet descriptor_set{};
            VkDescriptorSetAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorSetCount = 1;
            alloc_info.descriptorPool = gfx.desc_pool;
            alloc_info.pSetLayouts = &desc_layout;
            vk_check(vkAllocateDescriptorSets(gfx.device, &alloc_info, &descriptor_set));

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            // Set 1 binding 0 = texture

            VkDescriptorImageInfo img_info{};
            img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            img_info.imageView = image_view;
            img_info.sampler = sampler;

            descriptor_write.dstSet = descriptor_set;
            descriptor_write.dstBinding = 0;
            descriptor_write.dstArrayElement = 0; // Array index 0
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pBufferInfo = nullptr;
            descriptor_write.pImageInfo = &img_info;
            descriptor_write.pTexelBufferView = nullptr;
            vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

            return descriptor_set;
        };

        auto calculate_uv = [](const GeometryData &geometry_data, const components::AABBWallData &wall_data, glm::vec3 position, glm::vec3 tangent, glm::vec3 bi_tangent) -> glm::vec2
        {
            glm::vec2 uv{};
            uv.x = glm::dot(position, tangent) / geometry_data.image_width * wall_data.uv_scale.x + wall_data.uv_offset.x / geometry_data.image_width;
            uv.y = glm::dot(position, bi_tangent) / geometry_data.image_height * wall_data.uv_scale.y + wall_data.uv_offset.y / geometry_data.image_height;
            return uv;
        };

        auto new_geometry_data = [this, &load_image, &create_descriptor_set](components::Map &map, const std::string &texture)
        {
            if (image_path_to_geometry_data_map.find(texture) == image_path_to_geometry_data_map.end())
            {
                GeometryData new_data;
                new_data.vertex_count = 0;
                new_data.vertex_buffer = nullptr;
                new_data.image = load_image(texture, new_data.image_width, new_data.image_height);
                new_data.vertices = {};
                new_data.image_descriptor_set = create_descriptor_set(new_data.image->get_image_view(), new_data.image->get_sampler());

                image_path_to_geometry_data_map.insert({texture, std::move(new_data)});
            }
        };

        for (auto &map : maps)
        {
            for (const auto &aabb_wall : map->aabb_walls)
            {
                glm::vec3 max = aabb_wall.a + aabb_wall.b;
                glm::vec3 min = aabb_wall.a - aabb_wall.b;
                glm::vec3 n_front = {0, 0, -1};
                glm::vec3 n_back = {0, 0, 1};
                glm::vec3 n_top = {0, 1, 0};
                glm::vec3 n_bottom = {0, -1, 0};
                glm::vec3 n_left = {-1, 0, 0};
                glm::vec3 n_right = {1, 0, 0};

                // Front
                new_geometry_data(*map, aabb_wall.front.texture);
                {
                    GeometryData &geometry_data = image_path_to_geometry_data_map.at(aabb_wall.front.texture);
                    glm::vec3 tangent = {1, 0, 0};
                    glm::vec3 bi_tangent = {0, 1, 0};

                    geometry_data.vertices.push_back({min, n_front, calculate_uv(geometry_data, aabb_wall.front, min, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, max.y, min.z}, n_front, calculate_uv(geometry_data, aabb_wall.front, {min.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, max.y, min.z}, n_front, calculate_uv(geometry_data, aabb_wall.front, {max.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;

                    geometry_data.vertices.push_back({min, n_front, calculate_uv(geometry_data, aabb_wall.front, min, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, max.y, min.z}, n_front, calculate_uv(geometry_data, aabb_wall.front, {max.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, min.y, min.z}, n_front, calculate_uv(geometry_data, aabb_wall.front, {max.x, min.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;
                }
                // back
                new_geometry_data(*map, aabb_wall.back.texture);
                {
                    GeometryData &geometry_data = image_path_to_geometry_data_map.at(aabb_wall.back.texture);
                    glm::vec3 tangent = {1, 0, 0};
                    glm::vec3 bi_tangent = {0, 1, 0};

                    geometry_data.vertices.push_back({max, n_back, calculate_uv(geometry_data, aabb_wall.back, max, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, min.y, max.z}, n_back, calculate_uv(geometry_data, aabb_wall.back, {min.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, min.y, max.z}, n_back, calculate_uv(geometry_data, aabb_wall.back, {max.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;

                    geometry_data.vertices.push_back({max, n_back, calculate_uv(geometry_data, aabb_wall.back, max, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, max.y, max.z}, n_back, calculate_uv(geometry_data, aabb_wall.back, {min.x, max.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, min.y, max.z}, n_back, calculate_uv(geometry_data, aabb_wall.back, {min.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;
                }

                // Top
                new_geometry_data(*map, aabb_wall.top.texture);
                {
                    GeometryData &geometry_data = image_path_to_geometry_data_map.at(aabb_wall.top.texture);
                    glm::vec3 tangent = {1, 0, 0};
                    glm::vec3 bi_tangent = {0, 0, 1};

                    geometry_data.vertices.push_back({{min.x, max.y, min.z}, n_top, calculate_uv(geometry_data, aabb_wall.top, {min.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, max.y, max.z}, n_top, calculate_uv(geometry_data, aabb_wall.top, {min.x, max.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, max.y, min.z}, n_top, calculate_uv(geometry_data, aabb_wall.top, {max.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;

                    geometry_data.vertices.push_back({max, n_top, calculate_uv(geometry_data, aabb_wall.top, max, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, max.y, min.z}, n_top, calculate_uv(geometry_data, aabb_wall.top, {max.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, max.y, max.z}, n_top, calculate_uv(geometry_data, aabb_wall.top, {min.x, max.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;
                }

                // Bottom
                new_geometry_data(*map, aabb_wall.bottom.texture);
                {
                    GeometryData &geometry_data = image_path_to_geometry_data_map.at(aabb_wall.bottom.texture);
                    glm::vec3 tangent = {1, 0, 0};
                    glm::vec3 bi_tangent = {0, 0, 1};
                    geometry_data.vertices.push_back({{max.x, min.y, max.z}, n_bottom, calculate_uv(geometry_data, aabb_wall.bottom, {max.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, min.y, max.z}, n_bottom, calculate_uv(geometry_data, aabb_wall.bottom, {min.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, min.y, min.z}, n_bottom, calculate_uv(geometry_data, aabb_wall.bottom, {max.x, min.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;

                    geometry_data.vertices.push_back({min, n_bottom, calculate_uv(geometry_data, aabb_wall.bottom, min, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, min.y, min.z}, n_bottom, calculate_uv(geometry_data, aabb_wall.bottom, {max.x, min.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, min.y, max.z}, n_bottom, calculate_uv(geometry_data, aabb_wall.bottom, {min.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;
                }

                // right
                new_geometry_data(*map, aabb_wall.right.texture);
                {
                    GeometryData &geometry_data = image_path_to_geometry_data_map.at(aabb_wall.right.texture);
                    glm::vec3 tangent = {0, 0, 1};
                    glm::vec3 bi_tangent = {0, 1, 0};

                    geometry_data.vertices.push_back({{max.x, min.y, min.z}, n_right, calculate_uv(geometry_data, aabb_wall.right, {max.x, min.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, max.y, min.z}, n_right, calculate_uv(geometry_data, aabb_wall.right, {max.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, min.y, max.z}, n_right, calculate_uv(geometry_data, aabb_wall.right, {max.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;

                    geometry_data.vertices.push_back({{max.x, min.y, max.z}, n_right, calculate_uv(geometry_data, aabb_wall.right, {max.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{max.x, max.y, min.z}, n_right, calculate_uv(geometry_data, aabb_wall.right, {max.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({max, n_right, calculate_uv(geometry_data, aabb_wall.right, max, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;
                }

                // left
                new_geometry_data(*map, aabb_wall.left.texture);
                {
                    GeometryData &geometry_data = image_path_to_geometry_data_map.at(aabb_wall.left.texture);
                    glm::vec3 tangent = {0, 0, 1};
                    glm::vec3 bi_tangent = {0, 1, 0};

                    geometry_data.vertices.push_back({{min.x, max.y, max.z}, n_left, calculate_uv(geometry_data, aabb_wall.left, {min.x, max.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, max.y, min.z}, n_left, calculate_uv(geometry_data, aabb_wall.left, {min.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, min.y, max.z}, n_left, calculate_uv(geometry_data, aabb_wall.left, {min.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;

                    geometry_data.vertices.push_back({{min.x, min.y, max.z}, n_left, calculate_uv(geometry_data, aabb_wall.left, {min.x, min.y, max.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({{min.x, max.y, min.z}, n_left, calculate_uv(geometry_data, aabb_wall.left, {min.x, max.y, min.z}, tangent, bi_tangent)});
                    geometry_data.vertices.push_back({min, n_left, calculate_uv(geometry_data, aabb_wall.left, min, tangent, bi_tangent)});
                    geometry_data.vertex_count += 3;
                }
            }

            for (auto &[image_path, geometry_data] : image_path_to_geometry_data_map)
            {
                geometry_data.vertex_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(MapVertex) * geometry_data.vertices.size(), geometry_data.vertices.data());
            }

            // Load static models
            for (const auto &model_path : map->static_models)
            {
                if (model_path_to_model_map.find(model_path.model_path) == model_path_to_model_map.end())
                {
                    log().info("Importing static model: {}", model_path.model_path);

                    tinygltf::Model model;
                    tinygltf::TinyGLTF loader;
                    std::string err;
                    std::string warn;
                    loader.SetImageLoader(tinygltf::LoadImageData, nullptr);
                    loader.SetImageWriter(tinygltf::WriteImageData, nullptr);
                    if (!loader.LoadBinaryFromFile(&model, &err, &warn, model_path.model_path))
                    {
                        log().critical("Failed to import scene: {} | {} | {}", model_path.model_path, warn, err);
                        throw SceneException{"Failed to import scene: " + model_path.model_path + "| " + warn + "| " + err};
                    }

                    const tinygltf::Mesh &mesh = model.meshes.at(0);
                    auto extract_buffer_from_accessor = [&](const tinygltf::Accessor &accessor)
                        -> std::vector<unsigned char>
                    {
                        const auto &buffer_view = model.bufferViews.at(accessor.bufferView);
                        const auto &buffer = model.buffers.at(buffer_view.buffer);

                        std::vector<unsigned char> result(buffer_view.byteLength);
                        std::memcpy(result.data(), buffer.data.data() + buffer_view.byteOffset, buffer_view.byteLength);

                        return result;
                    };

                    auto extract_indices_from_primitive = [&](const tinygltf::Primitive &primitive, std::vector<uint32_t> &indices)
                    {
                        const auto &accessor = model.accessors.at(primitive.indices);
                        auto index_vector = extract_buffer_from_accessor(accessor);
                        switch (accessor.componentType)
                        {
                        case 5123:
                        {
                            for (uint32_t i = 0; i < accessor.count; i++)
                            {
                                const uint16_t *index = (const uint16_t *)&index_vector.at(i * sizeof(uint16_t));
                                indices.push_back(uint32_t(*index));
                            }
                            break;
                        }

                        case 5125:
                        {
                            for (uint32_t i = 0; i < accessor.count; i++)
                            {
                                const uint32_t *index = (const uint32_t *)&index_vector.at(i * sizeof(uint32_t));
                                indices.push_back(uint32_t(*index));
                            }
                            break;
                        }

                        default:
                            throw SceneException{"Invalid index component type !"};
                        }
                    };

                    auto extract_data_from_primitive = [&](const tinygltf::Primitive &primitive,
                                                           std::vector<glm::vec3> &positions,
                                                           std::vector<glm::vec3> &normals,
                                                           std::vector<glm::vec2> &texcoords)
                    {
                        if (primitive.attributes.find("POSITION") == primitive.attributes.end())
                        {
                            log().critical("Model vertex attribute must have POSITION attributes");
                            throw SceneException{"Model vertex attribute must have POSITION attributes"};
                        }

                        const auto &position_accessor = model.accessors.at(primitive.attributes.at("POSITION"));
                        std::vector<unsigned char> position_buffer = extract_buffer_from_accessor(position_accessor);

                        for (uint32_t i = 0; i < position_accessor.count; i++)
                        {
                            glm::vec3 position{};
                            std::memcpy(&position.x, position_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                            positions.push_back(position);
                        }

                        if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                        {
                            const auto &accessor = model.accessors.at(primitive.attributes.at("NORMAL"));
                            auto buffer = extract_buffer_from_accessor(accessor);
                            for (uint32_t i = 0; i < accessor.count; i++)
                            {
                                glm::vec3 normal = *(glm::vec3 *)&buffer.at(i * sizeof(normal));
                                normals.push_back(normal);
                            }
                        }
                        else
                        {
                            // Fill it with zeroes
                            auto vertex_count = position_accessor.count;
                            normals.resize(vertex_count);
                            std::memset(normals.data(), 0, vertex_count * sizeof(glm::vec3));
                        }

                        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                        {
                            const auto &accessor = model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
                            auto buffer = extract_buffer_from_accessor(accessor);

                            for (uint32_t i = 0; i < accessor.count; i++)
                            {
                                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                                {
                                    glm::vec2 tex_coord = *(glm::vec2 *)&buffer.at(i * sizeof(tex_coord));
                                    texcoords.push_back(tex_coord);
                                }
                                else
                                {
                                    log().info("TEXCOORD_0: TODO !");
                                    throw SceneException{};
                                }
                            }
                        }
                        else
                        {
                            // Fill with zeroes
                            auto vertex_count = position_accessor.count;
                            texcoords.resize(vertex_count);
                            std::memset(texcoords.data(), 0, vertex_count * sizeof(glm::vec<2, float>));
                        }
                    };

                    std::vector<MapVertex> vertices;
                    std::vector<glm::vec3> positions{};
                    std::vector<glm::vec3> normals{};
                    std::vector<glm::vec2> texcoords{};
                    std::vector<uint32_t> indices{};
                    for (const auto &primitive : mesh.primitives)
                    {
                        extract_indices_from_primitive(primitive, indices);
                        extract_data_from_primitive(primitive, positions, normals, texcoords);
                    }
                    vertices.reserve(positions.size());

                    for(uint32_t i = 0; i < positions.size(); i++)
                    {
                        vertices.push_back({positions.at(i), normals.at(i), texcoords.at(i)});
                    }

                    StaticModelData data;
                    data.vertex_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(MapVertex) * vertices.size(), vertices.data());
                    data.index_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t) * indices.size(), indices.data());
                    data.vertex_count = indices.size();

                    model_path_to_model_map.insert({model_path.model_path, std::move(data)});
                }
            }
        }
    }
    void MapRenderer::shutdown()
    {
        for (auto &map : maps)
        {
            for (const auto &[image_path, geometry_data] : image_path_to_geometry_data_map)
            {
                vkFreeDescriptorSets(gfx.device, gfx.desc_pool, 1, &geometry_data.image_descriptor_set);
            }
        }
        maps.clear();
    }
    void MapRenderer::add_map(std::shared_ptr<components::Map> map)
    {
        maps.push_back(std::move(map));
    }
    void MapRenderer::render(VkCommandBuffer cmd) const
    {
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = gfx.get_scaled_draw_extent().width;
        viewport.height = gfx.get_scaled_draw_extent().height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = gfx.get_scaled_draw_extent().width;
        scissor.extent.height = gfx.get_scaled_draw_extent().height;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &gfx.frame_datas[gfx.frame_index].global_set, 0, nullptr);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        for (const auto &map : maps)
        {
            for (const auto &[image_path, geometry_data] : image_path_to_geometry_data_map)
            {
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1, &geometry_data.image_descriptor_set, 0, nullptr);
                VkBuffer buffers[] =
                    {
                        geometry_data.vertex_buffer->get_buffer_handle()};
                VkDeviceSize offsets[] =
                    {0, 0, 0, 0, 0};

                vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(map->node.global_transform));
                vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
                vkCmdDraw(cmd, geometry_data.vertex_count, 1, 0, 0);
            }

            for(const auto& model_path : map->static_models)
            {
                const auto& model = model_path_to_model_map.at(model_path.model_path);
                 VkBuffer buffers[] =
                    {
                        model.vertex_buffer->get_buffer_handle()};
                VkDeviceSize offsets[] =
                    {0};

                vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(map->node.global_transform));
                vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
                vkCmdBindIndexBuffer(cmd, model.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, model.vertex_count, 1, 0, 0, 0);
            }
        }
    }
    void MapRenderer::render_depth(VkCommandBuffer cmd) const
    {
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = gfx.directional_light_shadow_map_resolution;
        viewport.height = gfx.directional_light_shadow_map_resolution;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = gfx.directional_light_shadow_map_resolution;
        scissor.extent.height = gfx.directional_light_shadow_map_resolution;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, depth_pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, depth_pipeline_layout, 0, 1, &gfx.frame_datas[gfx.frame_index].global_set, 0, nullptr);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        for (const auto &map : maps)
        {
            for (const auto &[image_path, geometry_data] : image_path_to_geometry_data_map)
            {
                VkBuffer buffers[] =
                    {
                        geometry_data.vertex_buffer->get_buffer_handle()};
                VkDeviceSize offsets[] =
                    {0, 0, 0, 0, 0};
                vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(map->node.global_transform));

                vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
                vkCmdDraw(cmd, geometry_data.vertex_count, 1, 0, 0);
            }
        }
    }

    void MapRenderer::create_pipeline()
    {
        // PER INSTANCE SET LAYOUT
        std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings{
            {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
        };

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = descriptor_layout_bindings.size();
        layout_ci.pBindings = descriptor_layout_bindings.data();
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(gfx.device, &layout_ci, nullptr, &desc_layout));
        std::vector<VkPushConstantRange> push_constants{
            VkPushConstantRange{
                VK_SHADER_STAGE_ALL,
                0,
                sizeof(glm::mat4x4)}};

        std::vector<VkDescriptorSetLayout> layouts = {gfx.global_set_layout, desc_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device, &pipeline_layout_info, nullptr, &pipeline_layout));

        // Create pipelie
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = sizeof(MapVertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},                           // position
            {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(MapVertex, normal)}, // normal
            {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(MapVertex, uv)},
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = vertex_bindings.size();
        vertex_input_info.pVertexBindingDescriptions = vertex_bindings.data();
        vertex_input_info.vertexAttributeDescriptionCount = vertex_attributes.size();
        vertex_input_info.pVertexAttributeDescriptions = vertex_attributes.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.primitiveRestartEnable = false;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        // multisampling defaulted to no multisampling (1 sample per pixel)
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        // no alpha to coverage either
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_TRUE;
        depth_stencil.minDepthBounds = 0.0f;
        depth_stencil.maxDepthBounds = 1.f;

        depth_stencil.front = {};
        depth_stencil.front.reference = STENCIL_VALUE;
        depth_stencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depth_stencil.front.compareMask = 0x00;
        depth_stencil.front.writeMask = 0xFF;
        depth_stencil.front.failOp = VK_STENCIL_OP_KEEP;
        depth_stencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depth_stencil.front.passOp = VK_STENCIL_OP_REPLACE;
        depth_stencil.back = depth_stencil.front;

        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 0;
        viewport.height = 0;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = 0;
        scissor.extent.height = 0;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.pScissors = &scissor;

        std::vector<VkPipelineColorBlendAttachmentState> blend_attachments =
            {
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = false,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},

                VkPipelineColorBlendAttachmentState{
                    .blendEnable = false,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},

                VkPipelineColorBlendAttachmentState{
                    .blendEnable = false,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},
            };

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = blend_attachments.size();
        color_blending.pAttachments = blend_attachments.data();

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
        VkShaderModule vertex_shader{};
        VkShaderModule fragment_shader{};
        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/map.vert.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/map.frag.spv");

        VkShaderModuleCreateInfo shader_module_ci = {};
        shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        // Vertex shader
        shader_module_ci.codeSize = vertex_binary.size();
        shader_module_ci.pCode = (uint32_t *)vertex_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &vertex_shader));
        shader_stage_ci.module = vertex_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        // Fragment shader
        shader_module_ci.codeSize = fragment_binary.size();
        shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &fragment_shader));
        shader_stage_ci.module = fragment_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        std::vector<VkDynamicState> dynamic_states =
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
        dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_ci.dynamicStateCount = dynamic_states.size();
        dynamic_state_ci.pDynamicStates = dynamic_states.data();

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = (uint32_t)pipeline_shader_stages.size();
        pipeline_info.pStages = pipeline_shader_stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state_ci;
        pipeline_info.pDepthStencilState = &depth_stencil;
        pipeline_info.layout = pipeline_layout;
        pipeline_info.renderPass = gfx.geometry_buffer->get_mainpass_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device, nullptr, 1, &pipeline_info, nullptr, &pipeline));

        vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    }

    void MapRenderer::create_depth_pipeline()
    {
        std::vector<VkPushConstantRange> push_constants{};

        std::vector<VkDescriptorSetLayout> layouts = {gfx.global_set_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device, &pipeline_layout_info, nullptr, &depth_pipeline_layout));

        // Create pipelie
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = sizeof(MapVertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0}, // position

        };

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = vertex_bindings.size();
        vertex_input_info.pVertexBindingDescriptions = vertex_bindings.data();
        vertex_input_info.vertexAttributeDescriptionCount = vertex_attributes.size();
        vertex_input_info.pVertexAttributeDescriptions = vertex_attributes.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.primitiveRestartEnable = false;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;
        rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        // multisampling defaulted to no multisampling (1 sample per pixel)
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        // no alpha to coverage either
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_FALSE;
        depth_stencil.front = {};
        depth_stencil.back = {};
        depth_stencil.minDepthBounds = 0.0f;
        depth_stencil.maxDepthBounds = 1.f;

        // Dummy viewport state
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 0;
        viewport.height = 0;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = 0;
        scissor.extent.height = 0;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.pScissors = &scissor;

        // setup dummy color blending. We arent using transparent objects yet
        // the blending is just "no blend", but we do write to the color attachment
        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 0;
        color_blending.pAttachments = nullptr;

        // build the actual pipeline
        // we now use all of the info structs we have been writing into into this one
        // to create the pipeline

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
        VkShaderModule vertex_shader{};
        VkShaderModule geometry_shader{};
        VkShaderModule fragment_shader{};

        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/map_shadow.vert.spv");
        auto geometry_binary = utils::file_path_to_binary("Core/shaders/compiled/shadow.geom.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/shadow.frag.spv");

        VkShaderModuleCreateInfo shader_module_ci = {};
        shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        // Vertex shader
        shader_module_ci.codeSize = vertex_binary.size();
        shader_module_ci.pCode = (uint32_t *)vertex_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &vertex_shader));
        shader_stage_ci.module = vertex_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        // Geometry shader
        shader_module_ci.codeSize = geometry_binary.size();
        shader_module_ci.pCode = (uint32_t *)geometry_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &geometry_shader));
        shader_stage_ci.module = geometry_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        // Fragment shader
        shader_module_ci.codeSize = fragment_binary.size();
        shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &fragment_shader));
        shader_stage_ci.module = fragment_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        std::vector<VkDynamicState> dynamic_states =
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
        dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_ci.dynamicStateCount = dynamic_states.size();
        dynamic_state_ci.pDynamicStates = dynamic_states.data();

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // pipeline_info.pNext = &render_info;
        pipeline_info.stageCount = (uint32_t)pipeline_shader_stages.size();
        pipeline_info.pStages = pipeline_shader_stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state_ci;
        pipeline_info.pDepthStencilState = &depth_stencil;
        pipeline_info.layout = depth_pipeline_layout;
        pipeline_info.renderPass = gfx.geometry_buffer->get_shadowpass_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device, nullptr, 1, &pipeline_info, nullptr, &depth_pipeline));

        vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device, geometry_shader, nullptr);
        vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    }

}