#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace JPH
{
    class BodyInterface;
    class BodyLockInterface;
    class PhysicsSystem;
    class TempAllocatorImpl;
    class JobSystemThreadPool;
    class BodyID;
    class Character;
    class Body;
}

namespace gage::phys
{
    class BPLayerInterface;
    class ObjectVsBroadPhaseLayerFilter;
    class ObjectLayerPairFilter;
    class Physics
    {
        
    public:
        Physics();
        ~Physics();

        void update(float delta);

        JPH::BodyInterface* get_body_interface();
        const JPH::BodyLockInterface *get_body_lock_interface() const;

        JPH::Character* create_character(const glm::vec3& position, const glm::quat& rotation);
        void destroy_character(JPH::Character* character);
    private:
        JPH::BodyInterface *p_body_interface{};
        const JPH::BodyLockInterface *p_body_lock_interface{};
        std::unique_ptr<JPH::PhysicsSystem> physics_system;
        std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
        std::unique_ptr<JPH::JobSystemThreadPool> job_system;
        std::unique_ptr<BPLayerInterface> broad_phase_layer_interface;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilter> object_vs_broadphase_layer_filter;
        std::unique_ptr<ObjectLayerPairFilter> object_vs_object_layer_filter;

        std::vector<std::unique_ptr<JPH::Character>> characters{};
    };
}