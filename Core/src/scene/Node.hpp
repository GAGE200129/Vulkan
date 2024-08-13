#pragma once

#include "components/IComponent.hpp"
#include "scene.hpp"

#include <vector>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp> 

namespace gage::scene
{
    class SceneGraph;
    class Node
    {
        friend class SceneGraph;
    public:
        Node(SceneGraph& scene, uint64_t id);
        ~Node();

        nlohmann::json to_json() const;

        void render_imgui();


        void add_component_ptr(components::IComponent* component);
        void* get_requested_component(const char* typeid_name);
        void* get_requested_component_recursive(const char* typeid_name);
        void get_requested_component_accumulate_recursive(const char* typeid_name, std::vector<void*>& out_components);

        
        Node* search_child_by_name(const std::string& name);

    private:  
        SceneGraph& scene;
    public:
        uint64_t id;
        std::string name;
        std::vector<components::IComponent*> component_ptrs;

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