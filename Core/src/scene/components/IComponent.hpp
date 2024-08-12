#pragma once

#include <nlohmann/json.hpp>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::scene
{
    class SceneGraph;
    class Node;
}



namespace gage::scene::components
{
    class IComponent
    {
    public:
        IComponent(SceneGraph& scene, Node& node) : scene(scene), node(node) {}
        virtual ~IComponent() = default;

        virtual nlohmann::json to_json() const = 0;

        //Debug
        virtual void render_imgui() = 0;
        virtual const char* get_name() const = 0;

    protected:
        SceneGraph& scene;
    public:
        Node& node;
    };
}