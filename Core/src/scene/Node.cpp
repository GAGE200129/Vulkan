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


    void Node::add_component_ptr(components::IComponent* component)
    {
        component_ptrs.push_back(component);
    }

    void Node::set_name(const std::string &name)
    {
        this->name = name;
    }

    uint64_t Node::get_id() const
    {
        return id;
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

    void Node::get_requested_component_accumulate_recursive(const char* typeid_name, std::vector<void*>& out_components)
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

    const std::vector<Node *> &Node::get_children() const
    {
        return children;
    }

    const std::string &Node::get_name() const
    {
        return name;
    }
    const glm::mat4x4 &Node::get_global_transform() const
    {
        return global_transform;
    }
    const glm::mat4x4 &Node::get_inverse_bind_transform() const
    {
        return inverse_bind_transform;
    }

    uint32_t Node::get_bone_id() const
    {
        return bone_id;
    }

    const glm::vec3 &Node::get_position()
    {
        return position;
    }
    const glm::vec3 &Node::get_scale()
    {
        return scale;
    }
    const glm::quat &Node::get_rotation()
    {
        return rotation;
    }

    void Node::set_position(const glm::vec3 &position)
    {
        this->position = position;
    }
    void Node::set_scale(const glm::vec3 &scale)
    {
        this->scale = scale;
    }
    void Node::set_rotation(const glm::quat &rotation)
    {
        this->rotation = rotation;
    }
}