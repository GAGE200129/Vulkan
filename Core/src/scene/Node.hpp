#pragma once

#include "components/IComponent.hpp"

#include <vector>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gage::scene
{
    class SceneGraph;
    class Node
    {
        friend class SceneGraph;
    public:
        Node(SceneGraph& scene, uint64_t id);
        ~Node();

        void set_name(const std::string& name);
        uint64_t get_id() const;
        const std::vector<Node*>& get_children() const;
        const std::string& get_name() const;

        const glm::mat4x4& get_global_transform() const;

    private:  
        SceneGraph& scene;
        uint64_t id;
        std::string name;
        std::vector<std::unique_ptr<components::IComponent>> components{};

        Node* parent = nullptr;
        std::vector<Node*> children{};

        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::mat4x4 global_transform{0.0f};
    };
}