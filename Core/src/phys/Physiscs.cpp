#include <pch.hpp>
#include "Physics.hpp"

#include "Layers.hpp"
#include "BoardPhaseLayers.hpp"
#include "BPLayerInterface.hpp"
#include "ObjectVsBroadPhaseLayerFilter.hpp"
#include "ObjectLayerPairFilter.hpp"

namespace gage::phys
{
    Physics::Physics() : 
                        physics_system(std::make_unique<JPH::PhysicsSystem>()),
                        temp_allocator(std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024)), // 10 MB
                        job_system(std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 1)),
                        broad_phase_layer_interface(std::make_unique<BPLayerInterface>()),
                        object_vs_broadphase_layer_filter(std::make_unique<ObjectVsBroadPhaseLayerFilter>()),
                        object_vs_object_layer_filter(std::make_unique<ObjectLayerPairFilter>())
    {
        // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint cMaxBodies = 1024;

        // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
        const uint cNumBodyMutexes = 0;

        // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
        // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
        // too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint cMaxBodyPairs = 1024;

        // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
        // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
        const uint cMaxContactConstraints = 1024;

        // Now we can create the actual physics system.
        physics_system->Init(cMaxBodies,
            cNumBodyMutexes,
            cMaxBodyPairs,
            cMaxContactConstraints,
            *broad_phase_layer_interface,
            *object_vs_broadphase_layer_filter,
            *object_vs_object_layer_filter
        );
        p_body_interface = &physics_system->GetBodyInterface();
    }
 
    Physics::~Physics()
    {

    }

    void Physics::update(float delta)
    {
        physics_system->Update(delta, 1, temp_allocator.get(), job_system.get());
    }
}