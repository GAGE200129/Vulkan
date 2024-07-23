#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace JPH
{
    class BodyInterface;
    class PhysicsSystem;
    class TempAllocatorImpl;
    class JobSystemThreadPool;
    class BodyID;
    class Character;
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

        JPH::Character* create_character(const glm::vec3& position, const glm::quat& rotation);
        void destroy_character(JPH::Character* character);
        //glm::mat4x4 character_get_transform(const JPH::Character* character); 
    private:
        JPH::BodyInterface *p_body_interface{};
        std::unique_ptr<JPH::PhysicsSystem> physics_system;
        std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
        std::unique_ptr<JPH::JobSystemThreadPool> job_system;
        std::unique_ptr<BPLayerInterface> broad_phase_layer_interface;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilter> object_vs_broadphase_layer_filter;
        std::unique_ptr<ObjectLayerPairFilter> object_vs_object_layer_filter;

        JPH::BodyID* floor{};
        JPH::BodyID* sphere{};

        std::vector<std::unique_ptr<JPH::BodyID>> bodies{};
        std::vector<std::unique_ptr<JPH::Character>> characters{};
    };
}