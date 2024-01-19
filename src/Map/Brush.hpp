#pragma once
#include <glm/vec3.hpp>

class Brush
{
public:
  Brush(const glm::vec3 &position)
  {
    mPlaneNormals[0] = {1, 0, 0};
    mPlaneDistance[0] = -1;

    mPlaneNormals[1] = {-1, 0, 0};
    mPlaneDistance[1] = 1;

    mPlaneNormals[2] = {0, 1, 0};
    mPlaneDistance[2] = -1;

    mPlaneNormals[3] = {0, -1, 0};
    mPlaneDistance[3] = 1;

    mPlaneNormals[4] = {0, 0, 1};
    mPlaneDistance[4] = -1;

    mPlaneNormals[5] = {0, 0, -1};
    mPlaneDistance[5] = 1;
  }

private:
  static constexpr size_t MAX_FACES = 6;
  glm::vec3 mPlaneNormals[MAX_FACES];
  float mPlaneDistance[MAX_FACES];
};