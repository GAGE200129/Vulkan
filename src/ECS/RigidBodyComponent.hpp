#pragma once

#include "Components.hpp"
#include "CollisionShapeBase.hpp"

#include <BulletDynamics/Dynamics/btRigidBody.h>

class RigidBodyComponent : public Component
{
public:
    RigidBodyComponent(float mass) : mMass(mass) {}
    void init() override;
    void update() override;
    void shutdown() noexcept override;

public:
    float mMass;
    btRigidBody *mBody;
    btMotionState *mMotionState;
    CollisionShapeBase *mShape;
};