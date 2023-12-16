#pragma once

#include "Components.hpp"

#include <BulletCollision/CollisionShapes/btCollisionShape.h>

class CollisionShapeBase : public Component
{
public:
  CollisionShapeBase(const glm::vec3& offset) : mColliderOffset(offset) {}
  virtual ~CollisionShapeBase() = default;

  void init() override {};
  void shutdown() noexcept override {};
public:
  btCollisionShape* mShape;
  glm::vec3 mColliderOffset;
};