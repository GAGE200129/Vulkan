#include <pch.hpp>
#include "SceneGraph.hpp"

#include "scene.hpp"

#include "components/MeshRenderer.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/PBRPipeline.hpp>

// namespace tinygltf
// {
//     bool LoadImageData(tinygltf::Image *image, const int /*image_idx*/,
//                        std::string * /*err*/, std::string * /*warn*/,
//                        int /*req_width*/, int /*req_height*/,
//                        const unsigned char *bytes, int size, void * /*user_data*/)
//     {

//         int w = 0, h = 0, comp = 0;
//         // Try to decode image header
//         if (stbi_info_from_memory(bytes, size, &w, &h, &comp))
//         {
//             stbi_uc *data = stbi_load_from_memory(bytes, size, &w, &h, &comp, STBI_rgb_alpha);

//             image->width = w;
//             image->height = h;
//             image->component = 4;
//             image->bits = 8;
//             image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
//             image->as_is = false;
//             image->image.resize(w * h * 4);
//             std::memcpy(image->image.data(), data, w * h * 4);

//             stbi_image_free(data);
//         }
//         else
//         {
//             unsigned char image_data[] =
//                 {
//                     0, 255, 0, 255, 255, 0, 0, 255,
//                     255, 255, 0, 255, 255, 0, 255, 255};
//             image->width = 2;
//             image->height = 2;
//             image->component = 4;
//             image->bits = 8;
//             image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
//             image->as_is = false;
//             image->image.resize(2 * 2 * 4);
//             std::copy(image_data, image_data + 2 * 2 * 4, image->image.begin());
//         }

//         return true;
//     }

//     bool WriteImageData(
//         const std::string * /* basepath */, const std::string * /* filename */,
//         const tinygltf::Image * /*image*/, bool /* embedImages */,
//         const tinygltf::FsCallbacks * /* fs_cb */, const tinygltf::URICallbacks * /* uri_cb */,
//         std::string * /* out_uri */, void * /* user_pointer */)
//     {
//         assert(false);
//     }
// }

namespace gage::scene
{
    SceneGraph::SceneGraph()
    {
        // Create root node
        id = 0;
        auto node = std::make_unique<Node>(*this, id);
        node->name = "Root";
        nodes.push_back(std::move(node));
        id++;
    }
    SceneGraph::~SceneGraph()
    {
    }

    void SceneGraph::update(float delta)
    {
        std::function<void(Node * node, glm::mat4x4 accumulated_transform)> traverse_scene_graph_recursive;
        traverse_scene_graph_recursive = [&](scene::Node *node, glm::mat4x4 accumulated_transform)
        {
            // Build node global transform
            accumulated_transform = glm::translate(accumulated_transform, node->position);
            accumulated_transform = glm::scale(accumulated_transform, node->scale);
            accumulated_transform *= glm::mat4x4(node->rotation);
            node->global_transform = accumulated_transform;

            

            for (const auto &component : node->components)
            {
                component->update(delta);
            }

            for (const auto &child : node->get_children())
            {
                traverse_scene_graph_recursive(child, accumulated_transform);
            }
        };

        traverse_scene_graph_recursive(nodes.at(0).get(), glm::mat4x4(1.0f));
    }

    void SceneGraph::render(gfx::Graphics &gfx, VkCommandBuffer cmd, VkPipelineLayout layout)
    {
        std::function<void(const Node *node)> traverse_scene_graph_recursive;
        traverse_scene_graph_recursive = [&](const scene::Node *node)
        {
            for (const auto &component : node->components)
            {
                component->render(gfx, cmd, layout);
            }

            for (const auto &child : node->get_children())
            {
                traverse_scene_graph_recursive(child);
            }
        };

        traverse_scene_graph_recursive(nodes.at(0).get());
    }

    const data::Model *SceneGraph::import_model(gfx::Graphics &gfx, const std::string &file_path, ImportMode mode)
    {
        log().info("Importing scene: {}", file_path);

        auto new_model = std::make_unique<data::Model>();
        new_model->name = file_path;

        tinygltf::Model gltf_model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        loader.SetImageLoader(tinygltf::LoadImageData, nullptr);
        loader.SetImageWriter(tinygltf::WriteImageData, nullptr);

        bool ret = false;
        switch (mode)
        {
        case ImportMode::Binary:
            ret = loader.LoadBinaryFromFile(&gltf_model, &err, &warn, file_path);
            break;
        case ImportMode::ASCII:
            ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, file_path);
            break;
        }

        if (!ret)
        {
            log().critical("Failed to import scene: {} | {} | {}", file_path, warn, err);
            throw SceneException{"Failed to import scene: " + file_path + "| " + warn + "| " + err};
        }
        new_model->root_node = gltf_model.scenes.at(gltf_model.defaultScene).nodes.at(0);

        // Import nodes
        new_model->nodes.reserve(gltf_model.nodes.size());
        for (const auto &gltf_node : gltf_model.nodes)
        {
            data::ModelNode model_node{};
            model_node.name = gltf_node.name;

            if (gltf_node.mesh > -1)
            {
                model_node.has_mesh = true;
                model_node.mesh_index = gltf_node.mesh;
            }

            glm::vec3 &position = model_node.position;
            glm::quat &rotation = model_node.rotation;
            glm::vec3 &scale = model_node.scale;

            // assert(gltf_node.matrix.size() == 0);

            // if (gltf_node.translation.size() != 0)
            // {

            // }

            if (gltf_node.translation.size() != 0)
            {
                position.x = gltf_node.translation.at(0);
                position.y = gltf_node.translation.at(1);
                position.z = gltf_node.translation.at(2);
            }

            if (gltf_node.scale.size() != 0)
            {
                scale.x = gltf_node.scale.at(0);
                scale.y = gltf_node.scale.at(1);
                scale.z = gltf_node.scale.at(2);
            }

            if (gltf_node.rotation.size() != 0)
            {
                rotation.x = gltf_node.rotation.at(0);
                rotation.y = gltf_node.rotation.at(1);
                rotation.z = gltf_node.rotation.at(2);
                rotation.w = gltf_node.rotation.at(3);
            }

            for (const auto &child : gltf_node.children)
            {
                model_node.children.push_back(child);
            }

            new_model->nodes.push_back(model_node);
        }

        // Process mesh
        new_model->meshes.reserve(gltf_model.meshes.size());
        for (const auto &gltf_mesh : gltf_model.meshes)
        {
            auto new_mesh = std::make_unique<data::ModelMesh>();
            this->process_model_mesh(gfx, gltf_model, gltf_mesh, new_mesh.get());
            new_model->meshes.push_back(std::move(new_mesh));
        }

        // Process material
        new_model->materials.reserve(gltf_model.materials.size());
        for (const auto &gltf_material : gltf_model.materials)
        {
            auto new_material = std::make_unique<data::ModelMaterial>();
            this->process_model_material(gfx, gltf_model, gltf_material, new_material.get());
            new_model->materials.push_back(std::move(new_material));
        }

        auto model_ptr = new_model.get();
        models.push_back(std::move(new_model));
        return model_ptr;
    }

    void SceneGraph::instanciate_model(const data::Model *model, glm::vec3 initial_position)
    {
        assert(model != nullptr);
        log().info("Instanciating model: {}", model->name);

        std::function<Node *(const data::Model &model, const data::ModelNode &model_node)> instanciate_node_recursive;
        instanciate_node_recursive = [&](const data::Model &model, const data::ModelNode &model_node) -> Node *
        {
            //if(model.root_node)
            
            Node *new_node = create_node();
            new_node->name = model_node.name;
            new_node->position = model_node.position;
            new_node->rotation = model_node.rotation;
            new_node->scale = model_node.scale;


            if (model_node.has_mesh)
            {
                new_node->components.push_back(std::make_unique<components::MeshRenderer>(*this, *new_node, model, *model.meshes.at(model_node.mesh_index).get()));
            }

            for (const uint32_t &node : model_node.children)
            {
                Node *child = instanciate_node_recursive(model, model.nodes.at(node));
                make_parent(new_node, child);
            }

            return new_node;
        };

        Node* new_node = instanciate_node_recursive(*model, model->nodes.at(model->root_node));
        new_node->position += initial_position;
    }

    // Every node created will be the child of the root node
    Node *SceneGraph::create_node()
    {
        auto node = std::make_unique<Node>(*this, id);
        auto node_ptr = node.get();

        auto root_node = nodes.at(0).get();
        node_ptr->parent = root_node;
        root_node->children.push_back(node_ptr);

        nodes.push_back(std::move(node));
        id++;

        return node_ptr;
    }

    void SceneGraph::make_parent(Node *parent, Node *child)
    {
        // Make sure to delete current child in the prev child's parent
        auto child_parent = child->parent;
        // get rid of children
        child_parent->children.erase(std::remove_if(child_parent->children.begin(), child_parent->children.end(),
                                                    [&child](const Node *node)
                                                    { return node == child; }));

        parent->children.push_back(child);
        child->parent = parent;
    }

    Node *SceneGraph::find_node(uint64_t id)
    {
        for (const auto &node : nodes)
        {
            if (node->id == id)
                return node.get();
        }

        return nullptr;
    }

    uint64_t SceneGraph::find_node_index(uint64_t id)
    {
        uint64_t index = 0;
        for (const auto &node : nodes)
        {
            if (node->id == id)
            {
                return index;
            }
            index++;
        }

        return 0;
    }

    const std::vector<std::unique_ptr<Node>> &SceneGraph::get_nodes() const
    {
        return nodes;
    }

    void SceneGraph::process_model_material(gfx::Graphics &gfx, const tinygltf::Model &gltf_model, const tinygltf::Material &gltf_material, data::ModelMaterial *material)
    {
        material->uniform_buffer_data.color = {
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(0),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(1),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(2),
            gltf_material.pbrMetallicRoughness.baseColorFactor.at(3),
        };

        gfx::data::ImageCreateInfo image_ci{VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT};

        // Has albedo texture ?
        const auto &albedo_texture_index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
        material->uniform_buffer_data.has_albedo = albedo_texture_index > -1;
        if (material->uniform_buffer_data.has_albedo)
        {
            const auto &image_src_index = gltf_model.textures.at(albedo_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;
            material->albedo_image.emplace(gfx, image.image.data(), image.width, image.height, size_in_bytes, image_ci);
        }

        // Has metalic roughness ?
        const auto &metalic_roughness_texture_index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material->uniform_buffer_data.has_metalic = metalic_roughness_texture_index > -1;
        if (material->uniform_buffer_data.has_metalic)
        {
            const auto &image_src_index = gltf_model.textures.at(metalic_roughness_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;
            material->metalic_roughness_image.emplace(gfx, image.image.data(), image.width, image.height, size_in_bytes, image_ci);
        }

        // Has normal map ?
        const auto &normal_texture_index = gltf_material.normalTexture.index;
        material->uniform_buffer_data.has_normal = normal_texture_index > -1;
        if (material->uniform_buffer_data.has_normal)
        {
            const auto &image_src_index = gltf_model.textures.at(normal_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;
            material->normal_image.emplace(gfx, image.image.data(), image.width, image.height, size_in_bytes, image_ci);
        }

        material->uniform_buffer.emplace(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(data::ModelMaterial::UniformBuffer), &material->uniform_buffer_data);
        material->descriptor_set = gfx.get_pbr_pipeline().allocate_instance_set(
            sizeof(data::ModelMaterial::UniformBuffer), material->uniform_buffer.value().get_buffer_handle(),
            material->albedo_image.has_value() ? material->albedo_image.value().get_image_view() : VK_NULL_HANDLE,
            material->albedo_image.has_value() ? material->albedo_image.value().get_sampler() : VK_NULL_HANDLE,
            material->metalic_roughness_image.has_value() ? material->metalic_roughness_image.value().get_image_view() : VK_NULL_HANDLE,
            material->metalic_roughness_image.has_value() ? material->metalic_roughness_image.value().get_sampler() : VK_NULL_HANDLE,
            material->normal_image.has_value() ? material->normal_image.value().get_image_view() : VK_NULL_HANDLE,
            material->normal_image.has_value() ? material->normal_image.value().get_sampler() : VK_NULL_HANDLE);
    }

    void SceneGraph::process_model_mesh(gfx::Graphics &gfx, const tinygltf::Model &gltf_model, const tinygltf::Mesh &gltf_mesh, data::ModelMesh *mesh)
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
                throw SceneException{"Invalid index component type !"};
            }
        };

        auto extract_data_from_primitive = [&](const Primitive &primitive, std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &texcoords)
        {
            if (primitive.attributes.find("POSITION") == primitive.attributes.end())
            {
                log().critical("Model vertex attribute must have POSITION attributes");
                throw SceneException{"Model vertex attribute must have POSITION attributes"};
            }

            const auto &position_accessor = gltf_model.accessors.at(primitive.attributes.at("POSITION"));
            std::vector<unsigned char> position_buffer = extract_buffer_from_accessor(position_accessor);
            std::vector<unsigned char> normal_buffer;
            std::vector<unsigned char> texcoord_buffer;

            if(primitive.attributes.find("NORMAL") != primitive.attributes.end())
            {
                normal_buffer = extract_buffer_from_accessor(gltf_model.accessors.at(primitive.attributes.at("NORMAL")));
            }
            else
            {
                //Fill it with zeroes
                auto vertex_count = position_accessor.count;
                normal_buffer.resize(vertex_count * sizeof(glm::vec3));
                std::memset(normal_buffer.data(), 0, normal_buffer.size());
            }

            if(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
            {
                texcoord_buffer = extract_buffer_from_accessor(gltf_model.accessors.at(primitive.attributes.at("TEXCOORD_0")));
            }
            else
            {
                //Fill with zeroes
                auto vertex_count = position_accessor.count;
                texcoord_buffer.resize(vertex_count * sizeof(glm::vec2));
                std::memset(texcoord_buffer.data(), 0, texcoord_buffer.size());
            }

            // bool texcoord_valid = texcoord_accessor.componentType == 5126; // float
            // if (!texcoord_valid)
            // {
            //     log().critical("Texture coord must be float !");
            //     throw SceneException{"Texture coord must be float !"};
            // }

            // Process
            for (uint32_t i = 0; i < position_accessor.count; i++)
            {
                glm::vec3 position{};
                glm::vec4 tangent{};
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

        // cull_radius = 0.0f;
        glm::vec3 max_pos{0, 0, 0};
        mesh->sections.reserve(gltf_mesh.primitives.size());
        for (const auto &primitive : gltf_mesh.primitives)
        {
            log().info("Loading mesh primitive: material: {}, indices: {}", primitive.material, primitive.indices);
            std::vector<glm::vec3> positions{};
            std::vector<glm::vec3> normals{};
            std::vector<glm::vec2> texcoords{};
            std::vector<uint32_t> indices{};
            extract_indices_from_primitive(primitive, indices);
            extract_data_from_primitive(primitive, positions, normals, texcoords);

            ////Calculate cull_radius
            // for(const auto& pos : positions)
            //{
            //     max_pos = glm::max(max_pos, glm::abs(pos));
            //     cull_radius = glm::length(max_pos);
            // }

            data::ModelMesh::MeshSection section{
                (uint32_t)indices.size(),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t) * indices.size(), indices.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * positions.size(), positions.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * normals.size(), normals.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec2) * texcoords.size(), texcoords.data()),
                (int32_t)primitive.material};
            mesh->sections.push_back(std::move(section));
        }
    }
}