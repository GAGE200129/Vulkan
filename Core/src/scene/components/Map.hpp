#pragma once

#include "IComponent.hpp"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gage::scene::components
{

    struct AABBWallData
    {
        std::string texture{};
        glm::vec2 uv_scale{1.0f};
        glm::vec2 uv_offset{0.0f};
    };
    struct AABBWall
    {
        glm::vec3 a{}, b{};

        AABBWallData front{};
        AABBWallData back{};
        AABBWallData left{};
        AABBWallData right{};
        AABBWallData top{};
        AABBWallData bottom{};
    };

    struct StaticModel
    {
        std::string model_path{};
        glm::vec3 offset{0, 0, 0};
        glm::quat rotation{0.0, 0.0, 0.0, 1.0};
    };

    struct PhysicsModel
    {
        std::string model_path{};
        glm::vec3 offset{0, 0, 0};
        glm::quat rotation{1.0, 0.0, 0.0, 0.0};
    };
    class Map final : public IComponent
    {   
    public:
        Map(SceneGraph& scene, Node& node);
        ~Map();

        nlohmann::json to_json() const final { return {{"type", get_name()}}; };
        
        void add_aabb_wall(AABBWall wall);
        void add_static_model(StaticModel model);
        void add_physics_model(PhysicsModel model);
        //Debug
        void render_imgui() {};
        const char* get_name() const { return "Map"; };
    public:
        std::vector<AABBWall> aabb_walls{};
        std::vector<StaticModel> static_models{};
        std::vector<std::unique_ptr<PhysicsModel>> physics_models{};
    };
}