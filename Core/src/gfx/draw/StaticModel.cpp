#include "StaticModel.hpp"

#include "../bind/VertexBuffer.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/TransformPS.hpp"
#include "../bind/Pipeline.hpp"
#include "../bind/Texture.hpp"
#include "../bind/DescriptorSet.hpp"
#include "../bind/UniformBuffer.hpp"
#include "../data/GUBO.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <tiny_gltf.h>

namespace gage::gfx::draw
{
    StaticModel::StaticModel(Graphics &gfx, std::string model_path)
    {
        bind::Pipeline *p_pipeline{};
        bind::Texture *p_texture{};
        if (!is_static_initialized())
        {
            struct Vertex
            {
                glm::vec3 position;
                glm::vec3 normal;
                glm::vec2 uv;
            };
            std::vector<VkVertexInputBindingDescription> bindings;
            std::vector<VkVertexInputAttributeDescription> attributes;

            // we will have just 1 vertex buffer binding, with a per-vertex rate
            VkVertexInputBindingDescription main_binding = {};
            main_binding.binding = 0;
            main_binding.stride = sizeof(Vertex);
            main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindings.push_back(main_binding);

            // Position will be stored at Location 0
            VkVertexInputAttributeDescription position_attribute = {};
            position_attribute.binding = 0;
            position_attribute.location = 0;
            position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
            position_attribute.offset = offsetof(Vertex, position);

            // Normal will be stored at Location 1
            VkVertexInputAttributeDescription normal_attribute = {};
            normal_attribute.binding = 0;
            normal_attribute.location = 1;
            normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
            normal_attribute.offset = offsetof(Vertex, normal);

            // Color will be stored at Location 2
            VkVertexInputAttributeDescription uv_attribute = {};
            uv_attribute.binding = 0;
            uv_attribute.location = 2;
            uv_attribute.format = VK_FORMAT_R32G32_SFLOAT;
            uv_attribute.offset = offsetof(Vertex, uv);

            attributes.push_back(position_attribute);
            attributes.push_back(normal_attribute);
            attributes.push_back(uv_attribute);

   

            tinygltf::Model model;
            tinygltf::TinyGLTF loader;
            std::string err;
            std::string warn;

            bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, model_path);

            if (!warn.empty())
            {
                logger.warn("Static model loader warn: " + warn);
            }

            if (!err.empty())
            {
                logger.error("Static model loader warn: " + warn);
                throw GraphicsException{};
            }

            if (!ret)
            {
                throw GraphicsException{};
            }

            std::vector<uint32_t> indices{};
            std::vector<Vertex> vertices{};
            try
            {
                using namespace tinygltf;
                auto extract_buffer_from_accessor = [](const Model &model, const Accessor &accessor)
                    -> std::vector<unsigned char>
                {
                    const auto &buffer_view = model.bufferViews.at(accessor.bufferView);
                    const auto &buffer = model.buffers.at(buffer_view.buffer);

                    std::vector<unsigned char> result(buffer_view.byteLength);
                    std::memcpy(result.data(), buffer.data.data() + buffer_view.byteOffset, buffer_view.byteLength);

                    return result;
                };

                auto extract_index_buffer_from_primitive = [extract_buffer_from_accessor](const Model &model, const Primitive &primitive, std::vector<uint32_t>& out_index_vec)
                {
                    const auto &accessor = model.accessors.at(primitive.indices);
                    auto index_vector = extract_buffer_from_accessor(model, accessor);
                    switch (accessor.componentType)
                    {
                    case 5123:
                    {
                        for (uint32_t i = 0; i < accessor.count; i++)
                        {
                            const uint16_t *index = (const uint16_t *)&index_vector.at(i * sizeof(uint16_t));
                            out_index_vec.push_back(uint32_t(*index));
                        }
                        break;
                    }

                    case 5125:
                    {
                        for (uint32_t i = 0; i < accessor.count; i++)
                        {
                            const uint32_t *index = (const uint32_t *)&index_vector.at(i * sizeof(uint32_t));
                            out_index_vec.push_back(uint32_t(*index));
                        }
                        break;
                    }

                    default:
                        throw GraphicsException{"Invalid index component type !"};
                    }
                };

                auto extract_vertex_buffer_from_primitive = [extract_buffer_from_accessor](const Model &model, const Primitive &primitive, std::vector<Vertex>& out_vertex_vec)
                {
                    bool attribute_valid = primitive.attributes.find("POSITION") != primitive.attributes.end();
                    attribute_valid |= primitive.attributes.find("NORMAL") != primitive.attributes.end();
                    attribute_valid |= primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
                    if(!attribute_valid)
                        throw GraphicsException{"Model vertex attribute must have POSITION, NORMAL, TEXCOORD_0 attributes"};

                    const auto& position_accessor = model.accessors.at(primitive.attributes.at("POSITION"));
                    auto position_buffer = extract_buffer_from_accessor(model, position_accessor);

                    const auto& normal_accessor = model.accessors.at(primitive.attributes.at("NORMAL"));
                    auto normal_buffer = extract_buffer_from_accessor(model, normal_accessor);

                    const auto& texcoord_accessor = model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
                    auto texcoord_buffer = extract_buffer_from_accessor(model, texcoord_accessor);

                    bool buffer_valid = position_accessor.count == normal_accessor.count && position_accessor.count == texcoord_accessor.count;
                    if(!buffer_valid)
                        throw GraphicsException{"Buffer invalid !"};

                    bool texcoord_valid = texcoord_accessor.componentType == 5126;//float
                    if(!texcoord_valid)
                        throw GraphicsException{"Texture coord must be float !"};

                    //Process position buffer
                    for(uint32_t i = 0; i < position_accessor.count; i++)
                    {
                        Vertex v{};
                        std::memcpy(&v.position.x, position_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                        std::memcpy(&v.normal.x, normal_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                        std::memcpy(&v.uv.x, texcoord_buffer.data() + i * sizeof(glm::vec2), sizeof(glm::vec2));
                        out_vertex_vec.push_back(v);
                    }
                };
                const Mesh &mesh = model.meshes[0];
                for (const auto &primitive : mesh.primitives)
                {
                    extract_index_buffer_from_primitive(model, primitive, indices);
                    extract_vertex_buffer_from_primitive(model, primitive, vertices);

                    
                }
            }
            catch (std::exception &e)
            {
                logger.error("Model might be corrupted: " + model_path);
                throw GraphicsException{};
            }

            VkDescriptorSetLayoutBinding descriptor_bindings[3]{};
            descriptor_bindings[0].binding = 0;
            descriptor_bindings[0].descriptorCount = 1;
            descriptor_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_bindings[0].stageFlags = VK_SHADER_STAGE_ALL;

            descriptor_bindings[1].binding = 1;
            descriptor_bindings[1].descriptorCount = 1;
            descriptor_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            descriptor_bindings[2].binding = 2;
            descriptor_bindings[2].descriptorCount = 1;
            descriptor_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkPushConstantRange push_constants[1]{};
            push_constants[0].offset = 0;
            push_constants[0].size = sizeof(float) * 16;
            push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            auto pipeline = std::make_unique<bind::Pipeline>();
            pipeline->set_vertex_layout(bindings, attributes);
            pipeline->set_push_constants(push_constants);
            pipeline->set_descriptor_set_bindings(descriptor_bindings);
            pipeline->set_vertex_shader("Core/shaders/compiled/colored_triangle.vert.spv", "main");
            pipeline->set_fragment_shader("Core/shaders/compiled/colored_triangle.frag.spv", "main");
            pipeline->set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            pipeline->set_polygon_mode(VK_POLYGON_MODE_FILL);
            pipeline->set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
            pipeline->set_multisampling_none();
            pipeline->set_blending_none();
            pipeline->enable_depth_test();
            pipeline->build(gfx);

            p_pipeline = pipeline.get();
            auto image = utils::file_path_to_image("res/textures/x.jpg", 4);
            auto texture = std::make_unique<bind::Texture>(gfx, image);

            p_texture = texture.get();

            add_static_bind(std::move(texture));
            add_static_bind(std::move(pipeline));
            add_static_index_buffer(std::make_unique<bind::IndexBuffer>(gfx, indices));
            add_static_bind(std::make_unique<bind::VertexBuffer>(gfx, 0, vertices.size() * sizeof(Vertex), vertices.data()));
        }
        else
        {
            this->index_buffer = search_static<bind::IndexBuffer>();
            p_pipeline = search_static<bind::Pipeline>();
            p_texture = search_static<bind::Texture>();
        }
        auto ubo = std::make_unique<bind::UniformBuffer>(gfx, sizeof(Material));
        ubo->update(&material);
        auto descriptor_set = std::make_unique<bind::DescriptorSet>(gfx, p_pipeline->get_layout(), p_pipeline->get_desc_set_layout());
        descriptor_set->set_buffer(gfx, 0, gfx.get_global_uniform_buffer().get(), gfx.get_global_uniform_buffer().get_size(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        descriptor_set->set_texture(gfx, 1, *p_texture);
        descriptor_set->set_buffer(gfx, 2, ubo->get(), ubo->get_size(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        add_bind(std::make_unique<bind::TransformPS>(gfx, p_pipeline->get_layout(), *this));
        add_bind(std::move(ubo));
        add_bind(std::move(descriptor_set));
    }

    glm::mat4 StaticModel::get_world_transform() const
    {
        return glm::mat4x4(1.0f);
    }

}