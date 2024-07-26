#pragma once

#include <Core/src/scene/components/CharacterController.hpp>
namespace gage::gfx::data
{
    class Camera;
}

namespace gage::scene::components
{
    class FPSCharacterController final : public CharacterController
    {

    public:
        FPSCharacterController(SceneGraph& scene, Node& node, phys::Physics& phys, gfx::data::Camera& camera);

        void init() final;
        void update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse) final;
        void shutdown() final;

        void render_imgui() final;
    private:
        gfx::data::Camera& camera;
        Node* head_node;

        float current_speed = {2.0f};
        float camera_y_offset{0.9f};
        float pitch{};
        float yaw{};
    };
}