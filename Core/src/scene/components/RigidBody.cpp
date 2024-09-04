#include <pch.hpp>
#include "RigidBody.hpp"

#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace gage::scene::components
{
    BoxShape::BoxShape(const glm::vec3 &center, const glm::vec3 &half_width) : center(center), half_width(half_width) {}
    JPH::ShapeSettings::ShapeResult BoxShape::generate_shape() const
    {
        JPH::BoxShapeSettings box_shape(JPH::Vec3(half_width.x, half_width.y, half_width.z));
        box_shape.SetEmbedded();
        JPH::OffsetCenterOfMassShapeSettings settings(JPH::Vec3(center.x, center.y, center.z), &box_shape);
        return settings.Create();
    }

    RigidBody::RigidBody(SceneGraph &scene, Node &node, std::unique_ptr<CollisionShape> shape) : IComponent(scene, node),
                                                                                                 shape(std::move(shape))
    {
    }
}