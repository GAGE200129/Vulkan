#include <pch.hpp>
#include "Physics.hpp"

#include "Layers.hpp"
#include "BoardPhaseLayers.hpp"
#include "BPLayerInterface.hpp"
#include "ObjectVsBroadPhaseLayerFilter.hpp"
#include "ObjectLayerPairFilter.hpp"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Character/Character.h>

namespace gage::phys
{
    using namespace JPH::literals;
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

        // Now create a dynamic body to bounce on the floor
        // Note that this uses the shorthand version of creating and adding a body to the world
        JPH::BodyCreationSettings floor_settings(new JPH::BoxShapeSettings(JPH::Vec3(100.0f, 1.0f, 100.0f)),
             JPH::RVec3(0.0_r, -1.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
        floor_settings.mFriction = 0.5f;
        auto new_floor = std::make_unique<JPH::BodyID>();
        *new_floor = p_body_interface->CreateAndAddBody(floor_settings, JPH::EActivation::DontActivate);
        floor = new_floor.get();
        bodies.push_back(std::move(new_floor));


    }
 
    Physics::~Physics()
    {
        for(const auto& body_id : bodies)
        {
            p_body_interface->RemoveBody(*body_id);
            p_body_interface->DestroyBody(*body_id);
        }
        bodies.clear();
    }

    void Physics::update(float delta)
    {
        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = 1;
        physics_system->Update(delta, cCollisionSteps, temp_allocator.get(), job_system.get());
    }

    JPH::Character* Physics::create_character(const glm::vec3& position, const glm::quat& rotation)
    {
        // Create 'player' character
        JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings();
        settings->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
        settings->mLayer = Layers::MOVING;
        settings->mShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.9f, 0.0), JPH::Quat(0, 0, 0, 1), JPH::CapsuleShapeSettings(0.9f, 0.1f).Create().Get()).Create().Get();
        settings->mFriction = 5.0f;
        
        std::unique_ptr<JPH::Character> character = std::make_unique<JPH::Character>(
            settings, 
            JPH::Vec3Arg(position.x, position.y, position.z),
            JPH::QuatArg(rotation.x, rotation.y, rotation.z, rotation.w),
            0,
            physics_system.get()
        );
        character->AddToPhysicsSystem(JPH::EActivation::Activate);

        

        JPH::Character* result = character.get();
        characters.push_back(std::move(character));
        return result;
    }

    void Physics::destroy_character(JPH::Character* character)
    {
        characters.erase(std::remove_if(characters.begin(), characters.end(), 
                       [&](const std::unique_ptr<JPH::Character>& c) { return c.get() == character; }), characters.end());
    }
}