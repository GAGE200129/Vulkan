#pragma once

#include "CharacterController.hpp"
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
        void update(float delta, const hid::Keyboard& keyboard) final;
        void shutdown() final;

        void render_imgui() final;
    private:
        gfx::data::Camera& camera;
        float camera_y_offset{0.9f};
    };
}