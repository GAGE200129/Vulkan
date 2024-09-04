#pragma once

#include "IComponent.hpp"

#include <glm/vec3.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>


namespace gage::phys
{
    class Physics;
   
}

namespace gage::scene::components
{
    class CollisionShape 
    {
    public:
        CollisionShape() = default;
        virtual ~CollisionShape() = default;

        virtual JPH::ShapeSettings::ShapeResult generate_shape() const = 0;
    };

    class BoxShape final : public CollisionShape
    {
    public:
        BoxShape(const glm::vec3& center, const glm::vec3& half_width);

        JPH::ShapeSettings::ShapeResult generate_shape() const override;
    private:
        glm::vec3 center;
        glm::vec3 half_width;
    };

    class RigidBody : public IComponent
    {
    public:
        RigidBody(SceneGraph& scene, Node& node, std::unique_ptr<CollisionShape> shape);

        nlohmann::json to_json() const final { return {}; };
        void render_imgui() override {};
        inline const char* get_name() const override { return "RigidBody"; };
    public:
        std::unique_ptr<CollisionShape> shape;
        JPH::BodyID body;
    };
}