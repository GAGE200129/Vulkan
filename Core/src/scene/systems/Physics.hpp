#pragma once

#include "../components/CharacterController.hpp"
#include "../components/Terrain.hpp"
#include "../components/Map.hpp"
#include "../components/RigidBody.hpp"
#include <vector>
#include <memory>

#include <glm/vec3.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>


namespace gage::scene
{
    class SceneGraph;
}


namespace gage::scene::systems
{
    namespace Layers 
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    };
    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr uint32_t NUM_LAYERS(2);
    };

    class ObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                assert(false);
                return false;
            }
        }
    };

    class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                assert(false);
                return false;
            }
        }
    };

    class BPLayerInterface final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterface()
        {
            // Create a mapping table from object to broad phase layer
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING; 
        
        }
        virtual uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            assert(inLayer < Layers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    class JoltIniter
    {
    public:
        JoltIniter();
        ~JoltIniter();
    };

    class Physics
    {
        friend class scene::SceneGraph;
    private:
        struct Terrain
        {
            JPH::BodyID height_map_body{};
            std::shared_ptr<components::Terrain> terrain_renderer;
        };

        struct Map
        {
            JPH::BodyID body{};
            std::shared_ptr<components::Map> map;
        };
    public:
        enum class GroundState
        {
            GROUND,
            AIR
        };
    public:
        Physics();
        ~Physics() = default;
        
        void init();
        void update(float);
        void shutdown();

        void add_character_controller(std::unique_ptr<components::CharacterController> character_controller);
        static void character_add_impulse(components::CharacterController* character, const glm::vec3& vel);
        static void character_set_velocity(components::CharacterController* character, const glm::vec3& vel);
        static glm::vec3 character_get_velocity(components::CharacterController* character);
        static GroundState character_get_ground_state(components::CharacterController* character);

        void add_terrain_renderer(std::shared_ptr<components::Terrain> terrain_renderer);
        void add_rigid_body(std::unique_ptr<components::RigidBody> rigid_body);
        void add_map(std::shared_ptr<components::Map> map);
    private:
        void extract_bounding_box(const std::string& file_path);
    private:
        JoltIniter jolt_initer;
        JPH::PhysicsSystem physics_system;
        JPH::TempAllocatorImpl temp_allocator;
        JPH::JobSystemThreadPool job_system;
        BPLayerInterface broad_phase_layer_interface;
        ObjectVsBroadPhaseLayerFilter object_vs_broadphase_layer_filter;
        ObjectLayerPairFilter object_vs_object_layer_filter;
        JPH::BodyInterface& body_interface;

        std::vector<std::unique_ptr<components::CharacterController>> character_controllers; 
        std::vector<Terrain> terrain_renderers;
        std::vector<Map> maps;
        std::vector<std::unique_ptr<components::RigidBody>> rigid_bodies;
    };
}