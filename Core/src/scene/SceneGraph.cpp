#include <pch.hpp>
#include "SceneGraph.hpp"

#include "scene.hpp"

#include "components/MeshRenderer.hpp"
#include "components/Animator.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/PBRPipeline.hpp>
#include <imgui/imgui.h>

#include <Core/src/mem.hpp>

namespace tinygltf
{
    bool LoadImageData(tinygltf::Image *image, const int /*image_idx*/,
                       std::string * /*err*/, std::string * /*warn*/,
                       int /*req_width*/, int /*req_height*/,
                       const unsigned char *bytes, int size, void * /*user_data*/)
    {

        int w = 0, h = 0, comp = 0;
        // Try to decode image header
        if (stbi_info_from_memory(bytes, size, &w, &h, &comp))
        {
            stbi_uc *data = stbi_load_from_memory(bytes, size, &w, &h, &comp, STBI_rgb_alpha);

            image->width = w;
            image->height = h;
            image->component = 4;
            image->bits = 8;
            image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            image->as_is = false;
            image->image.resize(w * h * 4);
            std::memcpy(image->image.data(), data, w * h * 4);

            stbi_image_free(data);
        }
        else
        {
            unsigned char image_data[] =
                {
                    0, 255, 0, 255, 255, 0, 0, 255,
                    255, 255, 0, 255, 255, 0, 255, 255};
            image->width = 2;
            image->height = 2;
            image->component = 4;
            image->bits = 8;
            image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            image->as_is = false;
            image->image.resize(2 * 2 * 4);
            std::copy(image_data, image_data + 2 * 2 * 4, image->image.begin());
        }

        return true;
    }

    bool WriteImageData(
        const std::string * /* basepath */, const std::string * /* filename */,
        const tinygltf::Image * /*image*/, bool /* embedImages */,
        const tinygltf::FsCallbacks * /* fs_cb */, const tinygltf::URICallbacks * /* uri_cb */,
        std::string * /* out_uri */, void * /* user_pointer */)
    {
        assert(false);
    }
}

namespace gage::scene
{
    SceneGraph::SceneGraph(gfx::Graphics &gfx, phys::Physics &phys, const gfx::data::Camera &camera) : gfx(gfx),
                                                                                                       renderer(gfx, camera),
                                                                                                       physics(phys)
    {
        // Create root node
        id = 0;
        auto node = std::make_unique<Node>(*this, id);
        node->name = ROOT_NAME;
        nodes.push_back(std::move(node));
        id++;
    }
    SceneGraph::~SceneGraph()
    {
        generic.shutdown();
        renderer.shutdown();
        animation.shutdown();
        physics.shutdown();
    }

    void SceneGraph::save(const std::string &file_path)
    {
        std::ofstream file(file_path);
        if (file.is_open())
        {
            file << nodes.at(0).get()->to_json().dump(2);
            file.close();
        }
    }

    void SceneGraph::init()
    {
        renderer.init();
        animation.init();
        physics.init();
        generic.init();
    }

    void SceneGraph::build_node_transform()
    {
        std::function<void(Node * node, glm::mat4x4 accumulated_transform)> traverse_scene_graph_recursive;
        traverse_scene_graph_recursive = [&](scene::Node *node, glm::mat4x4 accumulated_transform)
        {
            // Build node global transform
            accumulated_transform = glm::translate(accumulated_transform, node->position);
            accumulated_transform = glm::scale(accumulated_transform, node->scale);
            accumulated_transform *= glm::mat4x4(node->rotation);
            node->global_transform = accumulated_transform;

            for (const auto &child : node->get_children())
            {
                traverse_scene_graph_recursive(child, accumulated_transform);
            }
        };

        traverse_scene_graph_recursive(nodes.at(0).get(), glm::mat4x4(1.0f));
    }

    const data::Model &SceneGraph::import_model(const std::string &file_path, ImportMode mode)
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
        for (uint32_t i = 0; i < gltf_model.nodes.size(); i++)
        {
            const auto &gltf_node = gltf_model.nodes.at(i);

            data::ModelNode model_node{};
            model_node.name = gltf_node.name;
            model_node.bone_id = i;

            if (gltf_node.mesh > -1)
            {
                model_node.has_mesh = true;
                model_node.mesh_index = gltf_node.mesh;
            }

            if (gltf_node.skin > -1)
            {
                model_node.has_skin = true;
                model_node.skin_index = gltf_node.skin;
            }

            glm::vec3 &position = model_node.position;
            glm::quat &rotation = model_node.rotation;
            glm::vec3 &scale = model_node.scale;

            assert(gltf_node.matrix.size() == 0);

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
            auto new_mesh = data::ModelMesh{};
            this->process_model_mesh(gltf_model, gltf_mesh, new_mesh);
            new_model->meshes.push_back(std::move(new_mesh));
        }

        // Process material
        new_model->materials.reserve(gltf_model.materials.size());
        for (const auto &gltf_material : gltf_model.materials)
        {
            auto new_material = data::ModelMaterial{};
            this->process_model_material(gltf_model, gltf_material, new_material);
            new_model->materials.push_back(std::move(new_material));
        }

        // Process animations
        new_model->animations.reserve(gltf_model.animations.size());
        for (const auto &gltf_animation : gltf_model.animations)
        {
            auto new_animation = data::ModelAnimation{};
            this->process_model_animation(gltf_model, gltf_animation, new_animation);
            new_model->animations.push_back(std::move(new_animation));
        }

        // Process skins
        new_model->skins.reserve(gltf_model.skins.size());
        for (const auto &gltf_skin : gltf_model.skins)
        {
            auto new_skin = data::ModelSkin{};
            this->process_model_skin(gltf_model, gltf_skin, new_skin);
            new_model->skins.push_back(std::move(new_skin));
        }

        // Calculate inverse bind transform for skinning
        process_model_calculate_inverse_bind_transform(*new_model, new_model->nodes.at(new_model->root_node));

        auto model_ptr = new_model.get();
        models.push_back(std::move(new_model));
        return *model_ptr;
    }

    Node *SceneGraph::instanciate_model(const data::Model &model, glm::vec3 initial_position)
    {
        log().info("Instanciating model: {}", model.name);

        std::function<Node *(const data::Model &model, const data::ModelNode &model_node)> instanciate_node_recursive;
        instanciate_node_recursive = [&](const data::Model &model, const data::ModelNode &model_node) -> Node *
        {
            Node *new_node = create_node();
            new_node->name = model_node.name;
            new_node->position = model_node.position;
            new_node->rotation = model_node.rotation;
            new_node->scale = model_node.scale;
            new_node->inverse_bind_transform = model_node.inverse_bind_transform;
            new_node->bone_id = model_node.bone_id;

            if (model_node.has_mesh)
            {
                const data::ModelSkin *skin = nullptr;
                if (model_node.has_skin)
                {
                    skin = &model.skins.at(model_node.skin_index);
                }
                add_component(new_node, std::make_unique<components::MeshRenderer>(*this, *new_node, gfx, model, model.meshes.at(model_node.mesh_index), skin));
            }

            for (const uint32_t &node : model_node.children)
            {
                Node *child = instanciate_node_recursive(model, model.nodes.at(node));
                make_parent(new_node, child);
            }

            return new_node;
        };

        Node *new_node = instanciate_node_recursive(model, model.nodes.at(model.root_node));
        new_node->position += initial_position;

        return new_node;
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

    void SceneGraph::add_component(Node *node, std::unique_ptr<components::IComponent> component)
    {
        // Release component
        components::IComponent *ptr = component.release();
        node->add_component_ptr(ptr);

        if (std::strcmp(ptr->get_name(), "MeshRenderer") == 0)
        {
            renderer.add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer>(static_cast<components::MeshRenderer *>(ptr)));
        }
        else if (std::strcmp(ptr->get_name(), "TerrainRenderer") == 0) // this component will be shared
        {
            auto terrain_renderer = std::shared_ptr<components::TerrainRenderer>(static_cast<components::TerrainRenderer *>(ptr));
            renderer.add_terrain_renderer(terrain_renderer);
            physics.add_terrain_renderer(terrain_renderer);
        }
        else if (std::strcmp(ptr->get_name(), "Animator") == 0)
        {
            animation.add_animator(std::unique_ptr<components::Animator>(static_cast<components::Animator *>(ptr)));
        }
        else if (std::strcmp(ptr->get_name(), "CharacterController") == 0)
        {
            physics.add_character_controller(std::unique_ptr<components::CharacterController>(static_cast<components::CharacterController *>(ptr)));
        }
        else if (std::strcmp(ptr->get_name(), "Script") == 0)
        {
            generic.add_script(std::unique_ptr<components::Script>(static_cast<components::Script *>(ptr)));
        }
        else
        {
            log().critical("Unknown system for component: {}", ptr->get_name());
            throw SceneException{};
        }
    }

    const std::vector<std::unique_ptr<Node>> &SceneGraph::get_nodes() const
    {
        return nodes;
    }

    systems::Renderer &SceneGraph::get_renderer()
    {
        return renderer;
    }

    systems::Animation &SceneGraph::get_animation()
    {
        return animation;
    }
    systems::Physics &SceneGraph::get_physics()
    {
        return physics;
    }

    systems::Generic &SceneGraph::get_generic()
    {
        return generic;
    }

    void SceneGraph::process_model_material(const tinygltf::Model &gltf_model, const tinygltf::Material &gltf_material, data::ModelMaterial &material)
    {
        material.uniform_buffer_data.color = {
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
        material.uniform_buffer_data.has_albedo = albedo_texture_index > -1;
        if (material.uniform_buffer_data.has_albedo)
        {
            const auto &image_src_index = gltf_model.textures.at(albedo_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;

            image_ci.image_data = image.image.data();
            image_ci.width = image.width;
            image_ci.height = image.height;
            image_ci.size_in_bytes = size_in_bytes;
            material.albedo_image = std::make_unique<gfx::data::Image>(gfx, image_ci);
        }

        // Has metalic roughness ?
        const auto &metalic_roughness_texture_index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.uniform_buffer_data.has_metalic = metalic_roughness_texture_index > -1;
        if (material.uniform_buffer_data.has_metalic)
        {
            const auto &image_src_index = gltf_model.textures.at(metalic_roughness_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;

            image_ci.image_data = image.image.data();
            image_ci.width = image.width;
            image_ci.height = image.height;
            image_ci.size_in_bytes = size_in_bytes;
            material.metalic_roughness_image = std::make_unique<gfx::data::Image>(gfx, image_ci);
        }

        // Has normal map ?
        const auto &normal_texture_index = gltf_material.normalTexture.index;
        material.uniform_buffer_data.has_normal = normal_texture_index > -1;
        if (material.uniform_buffer_data.has_normal)
        {
            const auto &image_src_index = gltf_model.textures.at(normal_texture_index).source;
            const auto &image = gltf_model.images.at(image_src_index);
            size_t size_in_bytes = image.width * image.height * 4;

            image_ci.image_data = image.image.data();
            image_ci.width = image.width;
            image_ci.height = image.height;
            image_ci.size_in_bytes = size_in_bytes;
            material.normal_image = std::make_unique<gfx::data::Image>(gfx, image_ci);
        }

        material.uniform_buffer = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(data::ModelMaterial::UniformBuffer), &material.uniform_buffer_data);

        gfx::data::PBRPipeline::MaterialSetAllocInfo alloc_info{
            .size_in_bytes = sizeof(data::ModelMaterial::UniformBuffer),
            .buffer = material.uniform_buffer->get_buffer_handle(),
            .albedo_view = material.albedo_image ? material.albedo_image->get_image_view() : VK_NULL_HANDLE,
            .albedo_sampler = material.albedo_image ? material.albedo_image->get_sampler() : VK_NULL_HANDLE,
            .metalic_roughness_view = material.metalic_roughness_image ? material.metalic_roughness_image->get_image_view() : VK_NULL_HANDLE,
            .metalic_roughness_sampler = material.metalic_roughness_image ? material.metalic_roughness_image->get_sampler() : VK_NULL_HANDLE,
            .normal_view = material.normal_image ? material.normal_image->get_image_view() : VK_NULL_HANDLE,
            .normal_sampler = material.normal_image ? material.normal_image->get_sampler() : VK_NULL_HANDLE};
        material.descriptor_set = gfx.get_pbr_pipeline().allocate_material_set(alloc_info);
    }

    void SceneGraph::process_model_skin(const tinygltf::Model &gltf_model, const tinygltf::Skin &gltf_skin, data::ModelSkin &skin)
    {
        for (const auto &joint : gltf_skin.joints)
        {
            skin.joints.push_back(joint);
        }
    }

    void SceneGraph::process_model_mesh(const tinygltf::Model &gltf_model, const tinygltf::Mesh &gltf_mesh, data::ModelMesh &mesh)
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

            const auto &position_accessor = gltf_model.accessors.at(primitive.attributes.at("POSITION"));
            std::vector<unsigned char> position_buffer = extract_buffer_from_accessor(position_accessor);
            std::vector<unsigned char> normal_buffer;

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
            {
                normal_buffer = extract_buffer_from_accessor(gltf_model.accessors.at(primitive.attributes.at("NORMAL")));
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
                const auto &accessor = gltf_model.accessors.at(primitive.attributes.at("TEXCOORD_0"));
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

                const auto &accessor = gltf_model.accessors.at(primitive.attributes.at("JOINTS_0"));
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

                const auto &accessor = gltf_model.accessors.at(primitive.attributes.at("WEIGHTS_0"));
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

        mesh.sections.reserve(gltf_mesh.primitives.size());
        for (const auto &primitive : gltf_mesh.primitives)
        {
            std::vector<glm::vec3> positions{};
            std::vector<glm::vec3> normals{};
            std::vector<glm::vec2> texcoords{};
            std::vector<uint32_t> indices{};
            std::vector<glm::vec<4, uint16_t>> bone_ids{};
            std::vector<glm::vec4> bone_weights{};
            extract_indices_from_primitive(primitive, indices);
            extract_data_from_primitive(primitive, positions, normals, texcoords, bone_ids, bone_weights);

            data::ModelMesh::MeshSection section{
                (uint32_t)indices.size(),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t) * indices.size(), indices.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * positions.size(), positions.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec3) * normals.size(), normals.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec2) * texcoords.size(), texcoords.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec<4, uint16_t>) * bone_ids.size(), bone_ids.data()),
                std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(glm::vec4) * bone_weights.size(), bone_weights.data()),
                (int32_t)primitive.material,
                detect_skin(primitive)};
            mesh.sections.push_back(std::move(section));
        }
    }

    void SceneGraph::process_model_calculate_inverse_bind_transform(data::Model &model, data::ModelNode &root)
    {
        std::function<void(data::ModelNode & node, glm::mat4x4 accumulated_transform)> traverse_scene_graph_recursive;
        traverse_scene_graph_recursive = [&](data::ModelNode &node, glm::mat4x4 accumulated_transform)
        {
            // Build node model transform
            accumulated_transform = glm::translate(accumulated_transform, node.position);
            accumulated_transform = glm::scale(accumulated_transform, node.scale);
            accumulated_transform *= glm::mat4x4(node.rotation);
            node.inverse_bind_transform = glm::inverse(accumulated_transform); // get inverse

            for (const auto &child : node.children)
            {
                traverse_scene_graph_recursive(model.nodes.at(child), accumulated_transform);
            }
        };

        traverse_scene_graph_recursive(root, glm::mat4x4(1.0f));
    }

    void SceneGraph::process_model_animation(const tinygltf::Model &gltf_model, const tinygltf::Animation &gltf_animation, data::ModelAnimation &animation)
    {
        animation.name = gltf_animation.name;
        try
        {
            log().trace("Animation name: {}", gltf_animation.name);

            auto extract_buffer_from_accessor = [&](const tinygltf::Accessor &accessor)
                -> std::vector<unsigned char>
            {
                const auto &buffer_view = gltf_model.bufferViews.at(accessor.bufferView);
                const auto &buffer = gltf_model.buffers.at(buffer_view.buffer);

                std::vector<unsigned char> result(buffer_view.byteLength);
                std::memcpy(result.data(), buffer.data.data() + buffer_view.byteOffset, buffer_view.byteLength);

                return result;
            };

            animation.duration = std::numeric_limits<float>::min();
            for (const auto &gltf_channel : gltf_animation.channels)
            {
                const auto &gltf_sampler = gltf_animation.samplers.at(gltf_channel.sampler);
                const auto &time_point_accessor = gltf_model.accessors.at(gltf_sampler.input);
                auto time_points = extract_buffer_from_accessor(time_point_accessor); // .size() in bytes
                animation.duration = std::max(animation.duration, time_point_accessor.maxValues.at(0));
                if (gltf_channel.target_path.compare("translation") == 0)
                {
                    data::ModelAnimation::PositionChannel channel{};
                    channel.target_node = gltf_channel.target_node;
                    const auto &data_accessor = gltf_model.accessors.at(gltf_sampler.output);
                    auto data = extract_buffer_from_accessor(data_accessor); // .size() in bytes

                    channel.time_points.resize(time_point_accessor.count); // .size() in n * floats(4 bytes)
                    channel.positions.resize(data_accessor.count);         // .size() in n * glm::vec3 ( 12 bytes )
                    // log().info("Time point size: {} bytes, data size: {} bytes", time_points.size(),  data.size());

                    std::memcpy(channel.time_points.data(), time_points.data(), time_point_accessor.count * sizeof(float));
                    std::memcpy(channel.positions.data(), data.data(), data_accessor.count * sizeof(glm::vec3));

                    animation.pos_channels.push_back(std::move(channel));
                }
                else if (gltf_channel.target_path.compare("scale") == 0)
                {
                    data::ModelAnimation::ScaleChannel channel{};
                    channel.target_node = gltf_channel.target_node;
                    const auto &data_accessor = gltf_model.accessors.at(gltf_sampler.output);
                    auto data = extract_buffer_from_accessor(data_accessor); // .size() in bytes

                    channel.time_points.resize(time_point_accessor.count); // .size() in n * floats(4 bytes)
                    channel.scales.resize(data_accessor.count);            // .size() in n * glm::vec3 ( 12 bytes )
                    // log().info("Time point size: {} bytes, data size: {} bytes", time_points.size(),  data.size());

                    std::memcpy(channel.time_points.data(), time_points.data(), time_point_accessor.count * sizeof(float));
                    std::memcpy(channel.scales.data(), data.data(), data_accessor.count * sizeof(glm::vec3));

                    animation.scale_channels.push_back(std::move(channel));
                }

                else if (gltf_channel.target_path.compare("rotation") == 0)
                {
                    data::ModelAnimation::RotationChannel channel{};
                    channel.target_node = gltf_channel.target_node;
                    const auto &data_accessor = gltf_model.accessors.at(gltf_sampler.output);
                    auto data = extract_buffer_from_accessor(data_accessor); // .size() in bytes

                    channel.time_points.resize(time_point_accessor.count); // .size() in n * floats(4 bytes)
                    std::memcpy(channel.time_points.data(), time_points.data(), time_point_accessor.count * sizeof(float));

                    if (data_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                    {
                        channel.rotations.reserve(data_accessor.count);
                        for (uint32_t i = 0; i < data_accessor.count; i++)
                        {
                            float x = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 0);
                            float y = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 1);
                            float z = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 2);
                            float w = *(float *)&data.at(i * sizeof(glm::quat) + sizeof(float) * 3);
                            channel.rotations.push_back(glm::quat{w, x, y, z});
                        }
                    }
                    else
                    {
                        log().critical("Animation rotation channel is not in float ! TO DO LIST !");
                        throw SceneException{};
                    }

                    animation.rotation_channels.push_back(std::move(channel));
                }
            }
            log().trace("Animation duration: {}", animation.duration);
        }
        catch (std::out_of_range &e)
        {
            log().critical("Process model animation out of range caught !");
            throw SceneException{};
        }
    }

    void SceneGraph::render_imgui()
    {
        static Node *selected_node = nullptr;
        if (ImGui::Begin("SceneGraph"))
        {
            if (ImGui::Button("Save"))
            {
                save("res/maps/test.json");
            }

            if (ImGui::Button("New"))
            {
                create_node();
            }

            std::function<void(scene::Node * node)> browse_scene_graph_recursive;
            browse_scene_graph_recursive = [&browse_scene_graph_recursive](scene::Node *node)
            {
                const std::string &node_name = node->get_name();
                std::string id_string = std::to_string(node->get_id());
                std::string name = !node_name.empty() ? node_name + "|" + id_string : id_string;

                ImGuiTreeNodeFlags flags = 0;
                flags |= selected_node == node ? ImGuiTreeNodeFlags_Selected : 0;
                flags |= node->get_children().empty() ? ImGuiTreeNodeFlags_Leaf : 0;

                if (ImGui::TreeNodeEx(name.c_str(), flags))
                {
                    if (ImGui::IsItemClicked())
                    {
                        selected_node = node;
                    }
                    for (const auto &child : node->get_children())
                    {
                        browse_scene_graph_recursive(child);
                    }
                    ImGui::TreePop();
                }
            };

            browse_scene_graph_recursive(get_nodes().at(0).get());
        }
        ImGui::End();

        if (ImGui::Begin("Node Inspector"))
        {
            if (selected_node)
            {
                selected_node->render_imgui();
            }
        }
        ImGui::End();

        if (ImGui::Begin("Scene Graph systems"))
        {
            ImGui::Text("Renderer");
            ImGui::Text("Num mesh renderers: %lu", renderer.mesh_renderers.size());
            for(const auto& mesh_renderer : renderer.mesh_renderers)
            {
                ImGui::Text("unique_ptr: %p", mesh_renderer.get());
            }

            ImGui::Text("Num terrain renderers: %lu", renderer.terrain_renderers.size());
            for(const auto& terrain_renderer: renderer.terrain_renderers)
            {
                ImGui::Text("shared_ptr: %p", terrain_renderer.terrain_renderer.get());
            }
            ImGui::Separator();

            ImGui::Text("Animation");
            ImGui::Text("Num animators: %lu", animation.animators.size());
            for(const auto& animator : animation.animators)
            {
                ImGui::Text("unique_ptr: %p", animator.get());
            }
            ImGui::Separator();

            ImGui::Text("Physics");
            ImGui::Text("Num character: %lu", physics.character_controllers.size());
            for(const auto& character_controller : physics.character_controllers)
            {
                ImGui::Text("unique_ptr: %p", character_controller.get());
            }

            ImGui::Text("Num terrains: %lu", physics.terrain_renderers.size());
            for(const auto& terrain : physics.terrain_renderers)
            {
                ImGui::Text("shared_ptr: %p", terrain.terrain_renderer.get());
            }
            


        }
        ImGui::End();
    }
}