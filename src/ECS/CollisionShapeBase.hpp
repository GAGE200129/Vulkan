#pragma once

#include "Components.hpp"

#include <BulletCollision/CollisionShapes/btCollisionShape.h>

class CollisionShapeBase : public Component
{
public:
  CollisionShapeBase() = default;
  virtual ~CollisionShapeBase() = default;

  void init() override {};
  void shutdown() noexcept override {};
public:
  btCollisionShape* mShape;
};