#pragma once

#include "components/IComponent.hpp"
#include "scene.hpp"

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

        void render_imgui();

        void set_name(const std::string& name);
        uint64_t get_id() const;
        const std::vector<Node*>& get_children() const;
        const std::string& get_name() const;
        const glm::mat4x4& get_global_transform() const;
        
        const glm::mat4x4& get_inverse_bind_transform() const;
        uint32_t get_bone_id() const;

        void add_component(std::unique_ptr<components::IComponent> component);
        void* get_requested_component(const char* typeid_name);
        void* get_requested_component_recursive(const char* typeid_name);

        void set_position(const glm::vec3& position);
        void set_scale(const glm::vec3& scale);
        void set_rotation(const glm::quat& rotation);

        const glm::vec3& get_position();
        const glm::vec3& get_scale   ();
        const glm::quat& get_rotation();
        

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

        //Animation
        glm::mat4x4 inverse_bind_transform{1.0};
        uint32_t bone_id{};
    };
}