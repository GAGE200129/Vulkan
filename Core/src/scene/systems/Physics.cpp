#include <pch.hpp>
#include "Physics.hpp"

#include "../scene.hpp"
#include "../Node.hpp"
#include "../data/Model.hpp"

static void TraceImpl(const char *inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    // logger.trace(std::string(buffer));
}

namespace gage::scene::systems
{

    JoltIniter::JoltIniter()
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
    }
    JoltIniter::~JoltIniter()
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
    Physics::Physics() : jolt_initer(),
                         physics_system(),
                         temp_allocator(10 * 1024 * 1024),
                         job_system(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 1),
                         broad_phase_layer_interface(),
                         object_vs_broadphase_layer_filter(),
                         object_vs_object_layer_filter(),
                         body_interface(physics_system.GetBodyInterface())
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
        physics_system.Init(cMaxBodies,
                            cNumBodyMutexes,
                            cMaxBodyPairs,
                            cMaxContactConstraints,
                            broad_phase_layer_interface,
                            object_vs_broadphase_layer_filter,
                            object_vs_object_layer_filter);
    }

    void Physics::init()
    {
        for (auto &character_controller : character_controllers)
        {
            // Create 'player' character
            JPH::CharacterSettings settings;
            settings.mMaxSlopeAngle = JPH::DegreesToRadians(80.0f);
            settings.mLayer = Layers::MOVING;
            settings.mShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.0f, 0.0), JPH::Quat(0, 0, 0, 1),
                                                                  JPH::CapsuleShapeSettings(0.6f, 0.6f).Create().Get())
                                  .Create()
                                  .Get();
            settings.mFriction = 0.2f;

            // settings->mShape = JPH::CapsuleShapeSettings(1.8f, 0.3f).Create().Get();
            std::unique_ptr<JPH::Character> character = std::make_unique<JPH::Character>(
                &settings,
                JPH::Vec3Arg(character_controller->node.position.x, character_controller->node.position.y, character_controller->node.position.z),
                JPH::QuatArg(character_controller->node.rotation.x, character_controller->node.rotation.y, character_controller->node.rotation.z, character_controller->node.rotation.w),
                0, &physics_system);
            character->AddToPhysicsSystem(JPH::EActivation::Activate);

            character_controller->character = std::move(character);
        }

        for (auto &terrain_renderer : terrain_renderers)
        {
            // Init height map for physics

            JPH::HeightFieldShapeSettings shape_settings(terrain_renderer.terrain_renderer->height_map.data(), JPH::Vec3(0, 0, 0), JPH::Vec3(1, 1, 1), terrain_renderer.terrain_renderer->size);
            JPH::BodyCreationSettings setting(shape_settings.Create().Get(),
                                              JPH::RVec3(0.0, -1.0, 0.0),
                                              JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
            setting.mFriction = 0.2f;

            terrain_renderer.height_map_body = body_interface.CreateAndAddBody(setting, JPH::EActivation::DontActivate);
        }

        for (auto &map : maps)
        {
            JPH::StaticCompoundShapeSettings compound_shape_settings;
            for (const auto &aabb_wall : map.map->aabb_walls)
            {
                auto offset = aabb_wall.a + map.map->node.position;
                compound_shape_settings.AddShape(JPH::Vec3(offset.x, offset.y, offset.z), JPH::Quat::sIdentity(),
                                                 new JPH::BoxShape(JPH::Vec3(aabb_wall.b.x, aabb_wall.b.y, aabb_wall.b.z)));
            }

            // Static models
            for (const auto &static_model : map.map->static_models)
            {
                tinygltf::Model model;
                tinygltf::TinyGLTF loader;
                std::string err;
                std::string warn;
                if (!loader.LoadBinaryFromFile(&model, &err, &warn, static_model.model_path))
                {
                    log().critical("Physics failed to import scene: {} | {} | {}", static_model.model_path, warn, err);
                    throw SceneException{};
                }

                const tinygltf::Mesh &mesh = model.meshes.at(0);
                auto extract_buffer_from_accessor = [&](const tinygltf::Accessor &accessor)
                    -> std::vector<unsigned char>
                {
                    const auto &buffer_view = model.bufferViews.at(accessor.bufferView);
                    const auto &buffer = model.buffers.at(buffer_view.buffer);

                    std::vector<unsigned char> result(buffer_view.byteLength);
                    std::memcpy(result.data(), buffer.data.data() + buffer_view.byteOffset, buffer_view.byteLength);

                    return result;
                };

                std::vector<JPH::Vec3> positions;
                for (const auto &primitive : mesh.primitives)
                {
                    const auto &position_accessor = model.accessors.at(primitive.attributes.at("POSITION"));
                    std::vector<unsigned char> position_buffer = extract_buffer_from_accessor(position_accessor);

                    for (uint32_t i = 0; i < position_accessor.count; i++)
                    {
                        glm::vec3 position{};
                        std::memcpy(&position.x, position_buffer.data() + i * sizeof(glm::vec3), sizeof(glm::vec3));
                        positions.push_back(JPH::Vec3(position.x, position.y, position.z));
                    }
                }

                JPH::ConvexHullShapeSettings shape_setting(positions.data(), positions.size());

                glm::vec3 offset = static_model.offset + map.map->node.position;
                compound_shape_settings.AddShape(JPH::Vec3(offset.x, offset.y, offset.z), JPH::Quat::sIdentity(),
                                                 shape_setting.Create().Get());
            }

            JPH::BodyCreationSettings setting(compound_shape_settings.Create().Get(),
                                              JPH::RVec3(0.0, -1.0, 0.0),
                                              JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);
            map.body = body_interface.CreateAndAddBody(setting, JPH::EActivation::DontActivate);
        }

        for (auto &rigid_body : rigid_bodies)
        {
            JPH::BodyCreationSettings setting(rigid_body->shape->generate_shape().Get(),
                                              JPH::RVec3(rigid_body->node.position.x, rigid_body->node.position.y, rigid_body->node.position.z),
                                              JPH::Quat(rigid_body->node.rotation.x, rigid_body->node.rotation.y, rigid_body->node.rotation.z, rigid_body->node.rotation.w),
                                              JPH::EMotionType::Dynamic, Layers::MOVING);

            setting.mMassPropertiesOverride.mMass = 0.01;
            setting.mRestitution = 0.1;
            rigid_body->body = body_interface.CreateAndAddBody(setting, JPH::EActivation::Activate);
        }
    }

    void Physics::shutdown()
    {

        for (const auto &terrain_renderer : terrain_renderers)
        {
            body_interface.RemoveBody(terrain_renderer.height_map_body);
            body_interface.DestroyBody(terrain_renderer.height_map_body);
        }

        for (const auto &map : maps)
        {
            body_interface.RemoveBody(map.body);
            body_interface.DestroyBody(map.body);
        }
    }

    void Physics::update(float delta)
    {
        const int cCollisionSteps = 1;
        physics_system.Update(delta, cCollisionSteps, &temp_allocator, &job_system);

        for (auto &rigid_body : rigid_bodies)
        {
            // Extract body id
            JPH::Vec3 position = body_interface.GetCenterOfMassPosition(rigid_body->body);
            JPH::Quat rotation = body_interface.GetRotation(rigid_body->body);
            rigid_body->node.position = glm::vec3{position.GetX(), position.GetY(), position.GetZ()};
            rigid_body->node.rotation.x = rotation.GetX();
            rigid_body->node.rotation.y = rotation.GetY();
            rigid_body->node.rotation.z = rotation.GetZ();
            rigid_body->node.rotation.w = rotation.GetW();
        }
        for (auto &character_controller : character_controllers)
        {
            character_controller->character->PostSimulation(0.1f);
            auto position = character_controller->character->GetPosition(false);
            character_controller->node.position = {position.GetX(), position.GetY(), position.GetZ()};

            JPH::CharacterBase::EGroundState state = character_controller->character->GetGroundState();
            JPH::BodyLockWrite lock(physics_system.GetBodyLockInterface(), character_controller->character->GetBodyID());
            if (lock.Succeeded())
            {
                JPH::Body &body = lock.GetBody();

                switch (state)
                {
                case JPH::CharacterBase::EGroundState::InAir:
                {
                    body.GetMotionProperties()->SetLinearDamping(1.0f);
                    break;
                }

                case JPH::CharacterBase::EGroundState::NotSupported:
                case JPH::CharacterBase::EGroundState::OnGround:
                case JPH::CharacterBase::EGroundState::OnSteepGround:
                {
                    body.GetMotionProperties()->SetLinearDamping(5.0f);
                    break;
                }
                }
            }
        }
    }

    void Physics::add_character_controller(std::unique_ptr<components::CharacterController> character_controller)
    {
        character_controllers.push_back(std::move(character_controller));
    }

    void Physics::add_terrain_renderer(std::shared_ptr<components::Terrain> terrain_renderer)
    {
        Terrain terrain_renderer_additional_datas{};
        terrain_renderer_additional_datas.terrain_renderer = terrain_renderer;

        terrain_renderers.push_back(std::move(terrain_renderer_additional_datas));
    }

    void Physics::add_rigid_body(std::unique_ptr<components::RigidBody> rigid_body)
    {
        rigid_bodies.push_back(std::move(rigid_body));
    }

    void Physics::character_add_impulse(components::CharacterController *character, const glm::vec3 &vel)
    {
        character->character->AddImpulse(JPH::Vec3(vel.x, vel.y, vel.z));
    }
    void Physics::character_set_velocity(components::CharacterController *character, const glm::vec3 &vel)
    {
        character->character->SetLinearVelocity(JPH::Vec3(vel.x, vel.y, vel.z));
    }
    glm::vec3 Physics::character_get_velocity(components::CharacterController *character)
    {
        auto velocity = character->character->GetLinearVelocity(false);

        return glm::vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Physics::character_set_gravity_factor(components::CharacterController *character, const float gravity_factor) const
    {
        body_interface.SetGravityFactor(character->character->GetBodyID(), gravity_factor);
    }
    Physics::GroundState Physics::character_get_ground_state(components::CharacterController *character)
    {
        JPH::CharacterBase::EGroundState state = character->character->GetGroundState();
        switch (state)
        {
        case JPH::CharacterBase::EGroundState::InAir:
            return GroundState::AIR;
        case JPH::CharacterBase::EGroundState::OnGround:
            return GroundState::GROUND;
        default:
            return GroundState::GROUND;
        }

        return GroundState::GROUND;
    }

    void Physics::add_map(std::shared_ptr<components::Map> map)
    {
        Map additional_data{};
        additional_data.map = map;
        maps.push_back(std::move(additional_data));
    }

}