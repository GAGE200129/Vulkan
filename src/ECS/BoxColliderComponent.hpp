#pragma once

#include "CollisionShapeBase.hpp"

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <glm/vec3.hpp>

class BoxColliderComponent : public CollisionShapeBase
{
public:
    BoxColliderComponent(const glm::vec3 &offset, const glm::vec3 &halfWidth) : CollisionShapeBase(offset), mHalfWidth(halfWidth) {}
    virtual ~BoxColliderComponent() = default;

    void init() override
    {
        mShape = new btBoxShape(btVector3(mHalfWidth.x, mHalfWidth.y, mHalfWidth.z));
    }
    void shutdown() noexcept override
    {
        delete mShape;
    }

private:
    glm::vec3 mHalfWidth;
};