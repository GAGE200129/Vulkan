#pragma once

#include <Core/src/scene/components/Script.hpp>
#include <Core/src/scene/components/CharacterController.hpp>
#include <Core/src/scene/components/Animator.hpp>

namespace gage::gfx::data
{
    class Camera;
}

namespace gage::scene::systems
{
    class Physics;
}


class FPSCharacterController final : public gage::scene::components::Script
{
    enum class State
    {
        FLYING,
        NORMAL
    };
public:
    FPSCharacterController(gage::scene::SceneGraph &scene, gage::scene::Node &node, const gage::scene::systems::Physics& phys, gage::gfx::data::Camera &camera);

    void init() final;
    void update(float delta, const gage::hid::Keyboard &keyboard, const gage::hid::Mouse &mouse) final;
    void late_update(float delta, const gage::hid::Keyboard &keyboard, const gage::hid::Mouse &mouse) final;
    void shutdown() final;

    nlohmann::json to_json() const final;
private:
    const gage::scene::systems::Physics& phys;
    gage::gfx::data::Camera &camera;

    State current_state{State::NORMAL};

    gage::scene::components::CharacterController* character_controller{};
    gage::scene::components::Animator* animator{};
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
