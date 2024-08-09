#include <pch.hpp>
#include "SceneGraph.hpp"

#include "scene.hpp"

#include "components/MeshRenderer.hpp"
#include "components/Animator.hpp"

#include <Core/src/gfx/Graphics.hpp>
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
                                                                                                       renderer(gfx),
                                                                                                       terrain_renderer(gfx, camera),
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
        terrain_renderer.shutdown();
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
        terrain_renderer.init();
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
            new_model->nodes.emplace_back(gltf_model.nodes.at(i), i);
        }

        // Process mesh
        new_model->meshes.reserve(gltf_model.meshes.size());
        for (const auto &gltf_mesh : gltf_model.meshes)
        {
            new_model->meshes.emplace_back(gfx, gltf_model, gltf_mesh);
        }


        // Process material
        new_model->materials.reserve(gltf_model.materials.size());
        for (const auto &gltf_material : gltf_model.materials)
        {
            new_model->materials.emplace_back(gfx, renderer, gltf_model, gltf_material);
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


    void SceneGraph::add_component(Node *node, std::unique_ptr<components::IComponent> component)
    {
        // Release component
        components::IComponent *ptr = component.release();
        node->add_component_ptr(ptr);

        if (std::strcmp(ptr->get_name(), "MeshRenderer") == 0)
        {
            renderer.add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer>(static_cast<components::MeshRenderer *>(ptr)));
        }
        else if (std::strcmp(ptr->get_name(), "Terrain") == 0) // this component will be shared
        {
            auto terrain = std::shared_ptr<components::Terrain>(static_cast<components::Terrain *>(ptr));
            terrain_renderer.add_terrain(terrain);
            physics.add_terrain_renderer(terrain);
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
    systems::TerrainRenderer& SceneGraph::get_terrain_renderer()
    {
        return terrain_renderer; 
    }
    systems::Physics &SceneGraph::get_physics()
    {
        return physics;
    }

    systems::Generic &SceneGraph::get_generic()
    {
        return generic;
    }


    void SceneGraph::process_model_skin(const tinygltf::Model &gltf_model, const tinygltf::Skin &gltf_skin, data::ModelSkin &skin)
    {
        for (const auto &joint : gltf_skin.joints)
        {
            skin.joints.push_back(joint);
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

            ImGui::Text("Num terrain renderers: %lu", terrain_renderer.terrains.size());
            for(const auto& terrain: terrain_renderer.terrains)
            {
                ImGui::Text("shared_ptr: %p", terrain.terrain.get());
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