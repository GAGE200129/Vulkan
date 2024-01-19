#include "pch.hpp"
#include "Map.hpp"

void Map::addBrush(const glm::vec3& position)
{
  mBrushes.emplace_back(position);
}