#include "Mesh.hpp"

#include "../Exception.hpp"
#include "../Graphics.hpp"

#include "Model.hpp"

#include <tiny_gltf.h>

namespace gage::gfx::draw
{
    Mesh::Mesh(Graphics &gfx, Model& model, const tinygltf::Model &gltf_model, const tinygltf::Mesh &mesh) :
        gfx(gfx),
        model(model)
    {
        using namespace tinygltf;
        auto extract_buffer_from_accessor = [&](const Accessor &accessor)
            -> std::vector<unsigned char>
        {
            const auto &buffer_view = gltf_model.bufferViews.at(accessor.bufferView);
            const auto &buffer = gltf_model.buffers.at(buffer_view.buffer);

            std::vector<unsigned char> result(buffer_view.byteLength);
            std::memcpy(result.data(), buffer.data.data() + buffer_view.byteOffset, buffer_view.byteLength);

            return result;
        };

        auto extract_indices_from_primitive = [&](const Primitive &primitive, std::vector<uint32_t> &indices)
        {
            const auto &accessor = gltf_model.accessors.at(primitive.indices);
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
                throw GraphicsException{"Invalid index component type !"};
            }
        };

        auto extract_data_from_primitive = [&](const Primitive &primitive, std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &texcoords)
        {
            bool attribute_valid = primitive.attributes.find("POSITION") != primitive.attributes.end();
            attribute_valid |= primitive.attributes.find("NORMAL") != primitive.attributes.end();
            attribute_valid |= primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
            if (!attribute_valid)
                throw GraphicsException{"Model vertex attribute must have POSITION, NORMAL, TEXCOORD_0 attributes"};

            const auto &position_accessor = gltf_model.accessors.at(primitive.attributes.at("POSITION"));
            auto position_buffer = extract_buffer_from_accessor(position_accessor);

            const auto &normal_accessor = gltf_model.accessors.at(primitive.attributes.at("NORMAL"));
            auto normal_buffer = extract_buffer_from_accessor(normal_accessor);

            const auto &texcoord_accessor = gltf_model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
            auto texcoord_buffer = extract_buffer_from_accessor(texcoord_accessor);

            bool buffer_valid = position_accessor.count == normal_accessor.count && position_accessor.count == texcoord_accessor.count;
            if (!buffer_valid)
                throw GraphicsException{"Buffer invalid !"};

            bool texcoord_valid = texcoord_accessor.componentType == 5126; // float
            if (!texcoord_valid)
                throw GraphicsException{"Texture coord must be float !"};

            // Process
            for (uint32_t i = 0; i < position_accessor.count; i++)
            {
                glm::vec3 position{};
                glm::vec3 normal{};
                glm::vec2 uv{};
                std::memcpy(&position.x, position_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                std::memcpy(&normal.x, normal_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                std::memcpy(&uv.x, texcoord_buffer.data() + i * sizeof(glm::vec2), sizeof(glm::vec2));
                positions.push_back(position);
                normals.push_back(normal);
                texcoords.push_back(uv);
            }
        };

        this->sections.reserve(mesh.primitives.size());
        for (const auto &primitive : mesh.primitives)
        {
            std::vector<glm::vec3> positions{};
            std::vector<glm::vec3> normals{};
            std::vector<glm::vec2> texcoords{};
            std::vector<uint32_t> indices{};
            extract_indices_from_primitive(primitive, indices);
            extract_data_from_primitive(primitive, positions, normals, texcoords);

            this->sections.emplace_back(gfx, indices, positions, normals, texcoords, primitive.material);
        }
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::draw(VkCommandBuffer cmd) const
    {
        for (const auto &section : sections)
        {
            if(section.material_index < 0)
                continue;
            
            VkBuffer buffers[] =
                {
                    section.position_buffer.get_buffer_handle(),
                    section.normal_buffer.get_buffer_handle(),
                    section.texcoord_buffer.get_buffer_handle(),
                };
            VkDeviceSize offsets[] =
                {0, 0, 0};

            VkDescriptorSet desc_set = model.materials.at(section.material_index).get_desc_set();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                gfx.get_default_pipeline().get_pipeline_layout(),
                1, 
                1, &desc_set, 0, nullptr);
            
            vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
            vkCmdBindIndexBuffer(cmd, section.index_buffer.get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, section.vertex_count, 1, 0, 0, 0);
        }
    }

    Mesh::MeshSection::MeshSection(Graphics &gfx,
                                   const std::vector<uint32_t> &in_index_buffer,
                                   const std::vector<glm::vec3> &in_position_buffer,
                                   const std::vector<glm::vec3> &in_normal_buffer,
                                   const std::vector<glm::vec2> &in_texcoord_buffer,
                                   int32_t material_index) : 
                    vertex_count(in_index_buffer.size()),
                    index_buffer(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t) * in_index_buffer.size(), in_index_buffer.data()),
                    position_buffer(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * in_position_buffer.size(), in_position_buffer.data()),
                    normal_buffer(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * in_normal_buffer.size(), in_normal_buffer.data()),
                    texcoord_buffer(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec2) * in_texcoord_buffer.size(), in_texcoord_buffer.data()),
                    material_index(material_index)
    {
    }
}