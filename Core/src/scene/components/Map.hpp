#pragma once

#include "IComponent.hpp"

#include <glm/vec3.hpp>

namespace gage::scene::components
{
    struct AABBWall
    {
        glm::vec3 a{}, b{};
    };
    class Map final : public IComponent
    {
        
    public:
        Map(SceneGraph& scene, Node& node);
        ~Map();

        nlohmann::json to_json() const final { return {{"type", get_name()}}; };
        
        void add_aabb_wall(AABBWall wall);
        //Debug
        void render_imgui() {};
        const char* get_name() const { return "Map"; };
    public:
        std::vector<AABBWall> aabb_walls{};
    };
}