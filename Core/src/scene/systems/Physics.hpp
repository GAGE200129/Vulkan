#pragma once

#include "../components/CharacterController.hpp"
#include "../components/Terrain.hpp"
#include "../components/Map.hpp"
#include "../components/RigidBody.hpp"
#include <vector>
#include <memory>

#include <glm/vec3.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace gage::phys
{
    class Physics;
}

namespace gage::scene
{
    class SceneGraph;
}


namespace gage::scene::systems
{
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
        Physics(phys::Physics& phys);
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
        phys::Physics& phys;
        std::vector<std::unique_ptr<components::CharacterController>> character_controllers; 
        std::vector<Terrain> terrain_renderers;
        std::vector<Map> maps;
        std::vector<std::unique_ptr<components::RigidBody>> rigid_bodies;
    };
}