#pragma once

#include "Components.hpp"
#include "TransformComponent.hpp"

#include <BulletDynamics/Dynamics/btRigidBody.h>

class CharacterControllerComponent : public Component
{
public:
  void init() override;
  void update(float delta) override;
  void shutdown() noexcept override;

  static void registerLuaScript(lua_State* L);
public:
  static constexpr float RADIUS = 0.25f;
  static constexpr float RADIUS_THRESHOLD = 3;
  static constexpr float MAX_COS_GROUND = 1;
  static constexpr float HEIGHT = 1.7f;
  static constexpr float SPEED_DAMPING = 0.3f;
  static constexpr float WALK_ACCEL = 50.7f;
  static constexpr float MAX_VELOCITY = 10.0f;
public:
  bool mOnGround;
  glm::vec3 mGroundPoint;
  glm::vec3 mMoveDirection = {0, 0, 0};
  btRigidBody *mBody;
  btCollisionShape *mShape;
  btMotionState *mMotionState;
  TransformComponent *mTransform;
};