#pragma once

#include "IComponent.hpp"

#include <glm/vec3.hpp>

namespace gage::scene::components
{
    struct AABBWall
    {
        glm::vec3 a{}, b{};
    };
    class MapPiece final : public IComponent
    {
    public:
        MapPiece(SceneGraph& scene, Node& node);
        ~MapPiece();

        nlohmann::json to_json() const final { return {}; };

        //Debug
        void render_imgui() {};
        const char* get_name() const { return "MapPiece"; };
    private:
        std::vector<AABBWall> aabb_walls{};
    };
}