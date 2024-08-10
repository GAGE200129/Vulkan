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
    SceneGraph::SceneGraph(const gfx::Graphics &gfx, phys::Physics &phys, const gfx::data::Camera &camera) : gfx(gfx),
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

    const data::Model &SceneGraph::import_model(const std::string &file_path, data::ModelImportMode mode)
    {
        std::unique_ptr<data::Model> new_model = std::make_unique<data::Model>(gfx, renderer, file_path, mode);
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
                const std::vector<uint32_t> *joints = nullptr;
                if (model_node.has_skin)
                {
                    joints = &model.skins.at(model_node.skin_index);
                }
                add_component(new_node, std::make_unique<components::MeshRenderer>(*this, *new_node, gfx, model, model.meshes.at(model_node.mesh_index), joints));
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
    systems::TerrainRenderer &SceneGraph::get_terrain_renderer()
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
            for (const auto &mesh_renderer : renderer.mesh_renderers)
            {
                ImGui::Text("unique_ptr: %p", mesh_renderer.get());
            }

            ImGui::Text("Num terrain renderers: %lu", terrain_renderer.terrains.size());
            for (const auto &terrain : terrain_renderer.terrains)
            {
                ImGui::Text("shared_ptr: %p", terrain.terrain.get());
            }
            ImGui::Separator();

            ImGui::Text("Animation");
            ImGui::Text("Num animators: %lu", animation.animators.size());
            for (const auto &animator : animation.animators)
            {
                ImGui::Text("unique_ptr: %p", animator.get());
            }
            ImGui::Separator();

            ImGui::Text("Physics");
            ImGui::Text("Num character: %lu", physics.character_controllers.size());
            for (const auto &character_controller : physics.character_controllers)
            {
                ImGui::Text("unique_ptr: %p", character_controller.get());
            }

            ImGui::Text("Num terrains: %lu", physics.terrain_renderers.size());
            for (const auto &terrain : physics.terrain_renderers)
            {
                ImGui::Text("shared_ptr: %p", terrain.terrain_renderer.get());
            }
        }
        ImGui::End();
    }
}