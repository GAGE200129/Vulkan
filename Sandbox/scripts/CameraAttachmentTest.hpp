#pragma once

#include <Core/src/scene/components/IComponent.hpp>
namespace gage::gfx::data
{
    class Camera;
}

namespace gage::scene::components
{
    class CameraAttachmentTest final : public IComponent
    {

    public:
        CameraAttachmentTest(SceneGraph& scene, Node& node, gfx::data::Camera& camera);

        void init() final;
        void update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse) final;
        inline void shutdown() final {}

        inline void render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final {}
        inline void render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final {}

        inline void render_imgui() final {}
    private:
        gfx::data::Camera& camera;

        float current_speed = {2.0f};
        float camera_y_offset{0.9f};
        float pitch{};
        float yaw{};
    };
}