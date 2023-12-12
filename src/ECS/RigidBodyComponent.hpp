#pragma once

#include "Components.hpp"

#include <BulletDynamics/Dynamics/btRigidBody.h>

class RigidBodyComponent : public Component
{

public:
private:
  btRigidBody* mBody;
};