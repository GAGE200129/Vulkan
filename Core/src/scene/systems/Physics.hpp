#pragma once

#include "../components/CharacterController.hpp"
#include "../components/TerrainRenderer.hpp"
#include <vector>
#include <memory>

#include <glm/vec3.hpp>

namespace gage::phys
{
    class Physics;
}

namespace gage::scene::systems
{
    class Physics
    {
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

        void add_terrain_renderer(std::shared_ptr<components::TerrainRenderer> terrain_renderer);
    private:
        phys::Physics& phys;
        std::vector<std::unique_ptr<components::CharacterController>> character_controllers; 
        std::vector<std::shared_ptr<components::TerrainRenderer>> terrain_renderers;
    };
}