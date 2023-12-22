#pragma once

#include <glm/vec3.hpp>

class Face
{
  friend class Map;
public:
  Face(const glm::ivec3& p1, const glm::ivec3& p2, const glm::ivec3& p3) noexcept;
private:
  glm::ivec3 mP1, mP2, mP3;
  glm::dvec3 mNormal;
  double mDistance;
};