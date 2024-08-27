#include <pch.hpp>
#include "Map.hpp"

namespace gage::scene::components
{
    Map::Map(SceneGraph& scene, Node& node) :
        IComponent(scene, node)
    {

    }
    Map::~Map()
    {

    }
    void Map::add_aabb_wall(AABBWall wall)
    {
        aabb_walls.push_back(wall);
    }

    void Map::add_static_model(StaticModel model)
    {
        static_models.push_back(model);
    }

    void Map::add_physics_model(PhysicsModel model)
    {
        auto new_model = std::make_unique<PhysicsModel>();
        *new_model = model;// copy
        physics_models.push_back(std::move(new_model));
    }
}
