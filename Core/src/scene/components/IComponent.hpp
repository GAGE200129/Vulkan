#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::scene
{
    class SceneGraph;
    class Node;
}

namespace gage::hid
{
    class Keyboard;
    class Mouse;
}

namespace gage::scene::components
{
    class IComponent
    {
    public:
        IComponent(SceneGraph& scene, Node& node) : scene(scene), node(node) {}
        virtual ~IComponent() = default;

        virtual void init() = 0;
        virtual void update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse) = 0;
        virtual void late_update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse) = 0;
        virtual void render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) = 0;
        virtual void render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) = 0;
        virtual void shutdown() = 0;

        //Debug
        virtual void render_imgui() = 0;
        virtual const char* get_name() const = 0;
    protected:
        SceneGraph& scene;
        Node& node;
    };
}