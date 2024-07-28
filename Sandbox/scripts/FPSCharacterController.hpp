#pragma once

#include <Core/src/scene/components/Script.hpp>
#include <Core/src/scene/components/CharacterController.hpp>

namespace gage::gfx::data
{
    class Camera;
}

namespace gage::phys
{
    class Physics;
}


class FPSCharacterController final : public gage::scene::components::Script
{

public:
    FPSCharacterController(gage::scene::SceneGraph &scene, gage::scene::Node &node, gage::phys::Physics &phys, gage::gfx::data::Camera &camera);

    void init() final;
    void update(float delta, const gage::hid::Keyboard &keyboard, const gage::hid::Mouse &mouse) final;
    void late_update(float delta, const gage::hid::Keyboard &keyboard, const gage::hid::Mouse &mouse) final;
    void shutdown() final;


private:
    gage::gfx::data::Camera &camera;

    gage::scene::components::CharacterController* character_controller{};
    gage::scene::Node *head_node{};
    gage::scene::Node *spine1{};
    gage::scene::Node *spine{};
    gage::scene::Node *hips{};

    float current_speed = {2.0f};
    float camera_y_offset{0.9f};
    float pitch{};
    float yaw{};
    float target_roll{};
    float roll{};
};
