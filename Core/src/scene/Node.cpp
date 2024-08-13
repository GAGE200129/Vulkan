#include <pch.hpp>
#include "Node.hpp"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "SceneGraph.hpp"

namespace gage::scene
{
    Node::Node(SceneGraph &scene, uint64_t id) : scene(scene),
                                                 id(id)
    {
    }
    Node::~Node()
    {
    }

    nlohmann::json Node::to_json() const
    {
        nlohmann::json j;
        j["name"] = name;
        j["transform_pos_x"] = this->position.x;
        j["transform_pos_y"] = this->position.y;
        j["transform_pos_z"] = this->position.z;

        j["transform_scale_x"] = this->scale.x;
        j["transform_scale_y"] = this->scale.y;
        j["transform_scale_z"] = this->scale.z;

        j["transform_rotateion_x"] = this->rotation.x;
        j["transform_rotateion_y"] = this->rotation.y;
        j["transform_rotateion_z"] = this->rotation.z;
        j["transform_rotateion_w"] = this->rotation.w;

        j["bone_id"] = this->bone_id;

        std::array<float, 16> inverse_bind_matrix;
        for (uint32_t i = 0; i < 4; i++)
        {
            for (uint32_t j = 0; j < 4; j++)
            {
                inverse_bind_matrix[i + 4 * j] = inverse_bind_transform[i][j];
            }
        }
        j["inverse_bind_transform"] = inverse_bind_matrix;

        for (const auto &component : this->component_ptrs)
        {
            j["components"].push_back(component->to_json());
        }
        
        for (const auto &child : children)
        {
            j["children"].push_back(child->to_json());
        }

        
        return j;
    }

    void Node::render_imgui()
    {
        if (this->name.compare(SceneGraph::ROOT_NAME) == 0)
            return;

        ImGui::Text("Transform");
        ImGui::DragFloat3("position", &position.x, 0.1f);
        ImGui::DragFloat3("scale", &scale.x, 0.1f);
        ImGui::DragFloat4("rotation", glm::value_ptr(rotation), 0.01f, -1.0f, 1.0f);

        ImGui::Separator();

        for (auto &component : component_ptrs)
        {
            ImGui::Text("%s", component->get_name());
            component->render_imgui();
            ImGui::Separator();
        }
    }

    void Node::add_component_ptr(components::IComponent *component)
    {
        component_ptrs.push_back(component);
    }

   
    

    void *Node::get_requested_component(const char *typeid_name)
    {
        for (auto &component : component_ptrs)
        {
            log().trace("Searching for component: {}", typeid(*component).name());
            if (typeid(*component).name() == typeid_name)
            {
                return component;
            }
        }
        log().error("Unknown requested component: {}", typeid_name);
        return nullptr;
    }

    void *Node::get_requested_component_recursive(const char *typeid_name)
    {
        log().trace("get_requested_component_recursive: {}", this->name);
        auto component = this->get_requested_component(typeid_name);
        if (component != nullptr)
        {
            return component;
        }

        for (const auto &child : children)
        {
            log().trace("Searching for component in child: {}", child->name);
            return child->get_requested_component_recursive(typeid_name);
        }
        return nullptr;
    }

    void Node::get_requested_component_accumulate_recursive(const char *typeid_name, std::vector<void *> &out_components)
    {
        log().trace("get_requested_component_accumulate_recursive: {}", this->name);
        for (auto &component : this->component_ptrs)
        {
            if (typeid(*component).name() == typeid_name)
            {
                out_components.push_back(component);
            }
        }

        for (const auto &child : children)
        {
            child->get_requested_component_accumulate_recursive(typeid_name, out_components);
        }
    }

    Node *Node::search_child_by_name(const std::string &name)
    {
        if (this->name.compare(name) == 0)
        {
            return this;
        }
        for (const auto &child : this->children)
        {
            auto result = child->search_child_by_name(name);
            if (result)
            {
                return result;
            }
        }
        return nullptr;
    }
}