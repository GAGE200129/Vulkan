#pragma once

#include "Components.hpp"
#include "TransformComponent.hpp"
#include "CollisionShapeBase.hpp"

#include <BulletDynamics/Dynamics/btRigidBody.h>

class RigidBodyComponent : public Component
{
public:
  RigidBodyComponent(float mass) : mMass(mass) {}
  void init() override;
  void update(float delta) override;
  void shutdown() noexcept override;
private:
  float mMass;
  btRigidBody* mBody;
  btMotionState* mMotionState;
  TransformComponent* mTransform;
  CollisionShapeBase* mShape;
};