#include "pch.hpp"
#include "Face.hpp"

Face::Face(const glm::ivec3 &p1, const glm::ivec3 &p2, const glm::ivec3 &p3) noexcept 
  : mP1(p1), mP2(p2), mP3(p3)
{
  // Normal is on CW order
  glm::dvec3 p1_real = p1;
  glm::dvec3 p2_real = p2;
  glm::dvec3 p3_real = p3;
  mNormal = glm::normalize(glm::cross(p2_real - p1_real, p3_real - p1_real));
  mDistance = -(mNormal.x * p1_real.x + mNormal.y * p1_real.y + mNormal.z * p1_real.z);
  
}