#pragma once


#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct MapMesh
{
  std::vector<glm::vec3> position;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  std::vector<size_t> indices;
};