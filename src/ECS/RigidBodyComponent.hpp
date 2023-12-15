#pragma once

#include "Components.hpp"
#include "TransformComponent.hpp"

#include <BulletDynamics/Dynamics/btRigidBody.h>

class RigidBodyComponent : public Component
{
public:
  void init() override;
  void update(float delta) override;
  void shutdown() noexcept override;
private:
  btRigidBody* mBody;
  btMotionState* mMotionState;
  btCollisionShape* mShape;
  TransformComponent* mTransform;
};