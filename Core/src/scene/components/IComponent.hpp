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

namespace gage::scene::components
{
    class IComponent
    {
    public:
        IComponent(SceneGraph& scene, Node& node) : scene(scene), node(node) {}
        virtual ~IComponent() = default;

        virtual void init() = 0;
        virtual void update(float delta) = 0;
        virtual void render(gfx::Graphics& gfx, VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) = 0;
        virtual void shutdown() = 0;
    protected:
        SceneGraph& scene;
        Node& node;
    };
}