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
}
