#include <pch.hpp>
#include "ModelMesh.hpp"


#include <Core/src/gfx/Graphics.hpp>

#include "../scene.hpp"

namespace gage::scene::data
{
    ModelMeshPrimitive::~ModelMeshPrimitive()
    {

    }
    ModelMesh::ModelMesh(const gfx::Graphics& gfx, const tinygltf::Model& model, const tinygltf::Mesh& mesh)
    {
        using namespace tinygltf;
        auto extract_buffer_from_accessor = [&](const Accessor &accessor)
            -> std::vector<unsigned char>
        {
            const auto &buffer_view = model.bufferViews.at(accessor.bufferView);
            const auto &buffer = model.buffers.at(buffer_view.buffer);

            std::vector<unsigned char> result(buffer_view.byteLength);
            std::memcpy(result.data(), buffer.data.data() + buffer_view.byteOffset, buffer_view.byteLength);

            return result;
        };

        auto extract_indices_from_primitive = [&](const Primitive &primitive, std::vector<uint32_t> &indices)
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

        auto extract_data_from_primitive = [&](const Primitive &primitive,
                                               std::vector<glm::vec3> &positions,
                                               std::vector<glm::vec3> &normals,
                                               std::vector<glm::vec2> &texcoords,
                                               std::vector<glm::vec<4, uint16_t>> &bone_ids,
                                               std::vector<glm::vec4> &bone_weights)
        {
            if (primitive.attributes.find("POSITION") == primitive.attributes.end())
            {
                log().critical("Model vertex attribute must have POSITION attributes");
                throw SceneException{"Model vertex attribute must have POSITION attributes"};
            }

            const auto &position_accessor = model.accessors.at(primitive.attributes.at("POSITION"));
            std::vector<unsigned char> position_buffer = extract_buffer_from_accessor(position_accessor);
            std::vector<unsigned char> normal_buffer;

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
            {
                normal_buffer = extract_buffer_from_accessor(model.accessors.at(primitive.attributes.at("NORMAL")));
            }
            else
            {
                // Fill it with zeroes
                auto vertex_count = position_accessor.count;
                normal_buffer.resize(vertex_count * sizeof(glm::vec3));
                std::memset(normal_buffer.data(), 0, vertex_count * sizeof(glm::vec3));
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
                texcoords.resize(vertex_count * sizeof(glm::vec<2, float>));
                std::memset(texcoords.data(), 0, vertex_count * sizeof(glm::vec<2, float>));
            }

            if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
            {

                const auto &accessor = model.accessors.at(primitive.attributes.at("JOINTS_0"));
                auto buffer = extract_buffer_from_accessor(accessor);

                for (uint32_t i = 0; i < accessor.count; i++)
                {
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        glm::vec<4, uint16_t> bone_id = *(glm::vec<4, uint16_t> *)&buffer.at(i * sizeof(bone_id));
                        bone_ids.push_back(bone_id);
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        glm::vec<4, uint16_t> bone_id = *(glm::vec<4, uint8_t> *)&buffer.at(i * sizeof(glm::vec<4, uint8_t>));
                        bone_ids.push_back(bone_id);
                    }
                }
            }
            else
            {
                // Fill with zeroes
                auto vertex_count = position_accessor.count;
                bone_ids.resize(vertex_count * sizeof(glm::vec<4, uint16_t>));
                std::memset(bone_ids.data(), 0, vertex_count * sizeof(glm::vec<4, uint16_t>));
            }

            if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
            {

                const auto &accessor = model.accessors.at(primitive.attributes.at("WEIGHTS_0"));
                auto buffer = extract_buffer_from_accessor(accessor);

                for (uint32_t i = 0; i < accessor.count; i++)
                {
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                    {
                        glm::vec4 bone_weight = *(glm::vec4 *)&buffer.at(i * sizeof(bone_weight));
                        bone_weights.push_back(bone_weight);
                    }
                    else
                    {
                        log().info("WEIGHTS_0: TODO !");
                        throw SceneException{};
                    }
                }
            }
            else
            {
                // Fill with zeroes
                auto vertex_count = position_accessor.count;
                bone_weights.resize(vertex_count * sizeof(glm::vec4));
                std::memset(bone_weights.data(), 0, vertex_count * sizeof(glm::vec4));
            }

            // Process
            for (uint32_t i = 0; i < position_accessor.count; i++)
            {
                glm::vec3 position{};
                glm::vec3 normal{};
                std::memcpy(&position.x, position_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                std::memcpy(&normal.x, normal_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                positions.push_back(position);
                normals.push_back(normal);
            }
        };

        auto detect_skin = [](const tinygltf::Primitive &gltf_primitive) -> bool
        {
            return (gltf_primitive.attributes.find("WEIGHTS_0") != gltf_primitive.attributes.end()) && (gltf_primitive.attributes.find("JOINTS_0") != gltf_primitive.attributes.end());
        };

        primitives.reserve(mesh.primitives.size());
        for (const auto &primitive : mesh.primitives)
        {
            std::vector<glm::vec3> positions{};
            std::vector<glm::vec3> normals{};
            std::vector<glm::vec2> texcoords{};
            std::vector<uint32_t> indices{};
            std::vector<glm::vec<4, uint16_t>> bone_ids{};
            std::vector<glm::vec4> bone_weights{};
            extract_indices_from_primitive(primitive, indices);
            extract_data_from_primitive(primitive, positions, normals, texcoords, bone_ids, bone_weights);

            ModelMeshPrimitive new_primitive(
                (uint32_t)indices.size(),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t) * indices.size(), indices.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * positions.size(), positions.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * normals.size(), normals.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec2) * texcoords.size(), texcoords.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec<4, uint16_t>) * bone_ids.size(), bone_ids.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec4) * bone_weights.size(), bone_weights.data()),
                (int32_t)primitive.material,
                detect_skin(primitive)
            );
            primitives.emplace_back(std::move(new_primitive));
        }
    }   
    ModelMesh::~ModelMesh()
    {

    }
}