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
        if(this->name.compare(SceneGraph::ROOT_NAME) == 0)
            return;
        
        ImGui::Text("Transform");
        ImGui::DragFloat3("position", &position.x, 0.1f);
        ImGui::DragFloat3("scale", &scale.x, 0.1f);
        ImGui::DragFloat4("rotation", glm::value_ptr(rotation), 0.01f, -1.0f, 1.0f);

        ImGui::Separator();

        for(auto& component : components)
        {
            component->render_imgui();
        }
        
    }

    void Node::add_component(std::unique_ptr<components::IComponent> component)
    {
        components.push_back(std::move(component));
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
        for (auto &component : components)
        {
            log().trace("Searching for component: {}", typeid(*component.get()).name());
            if (typeid(*component.get()).name() == typeid_name)
            {
                return component.get();
            }
        }
        log().error("Unknown requested component: {}", typeid_name);
        return nullptr;
    }

    void* Node::get_requested_component_recursive(const char* typeid_name)
    {
        log().trace("get_requested_component_recursive: {}", this->name);
        for(const auto& child : children)
        {
            log().trace("Searching for component in child: {}", child->name);
            auto component = child->get_requested_component(typeid_name);
            if(component != nullptr)
            {
                return component;
            }

            return child->get_requested_component_recursive(typeid_name);
        }
        return nullptr;
    }

    Node* Node::search_child_by_name(const std::string& name) 
    {
        if(this->name.compare(name) == 0)
        {
            return this;
        }
        for(const auto& child : this->children)
        {
            auto result = child->search_child_by_name(name);
            if (result) {
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

    const glm::vec3& Node::get_position()
    {
        return position;
    }
    const glm::vec3& Node::get_scale   ()
    {
        return scale;
    }
    const glm::quat& Node::get_rotation()
    {
        return rotation;
    }


    void Node::set_position(const glm::vec3& position)
    {
        this->position = position;
    }
    void Node::set_scale(const glm::vec3& scale)
    {
        this->scale = scale;
    }
    void Node::set_rotation(const glm::quat& rotation)
    {
        this->rotation = rotation;
    }
}