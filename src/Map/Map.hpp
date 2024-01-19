#pragma once

#include "Brush.hpp"
#include "MapMesh.hpp"

#include <vector>

class Map
{
public:
  MapMesh getMapMesh();

  void addBrush(const glm::vec3& position);
public:
  std::vector<Brush> mBrushes;
};