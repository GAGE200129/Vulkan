#pragma once

#include "Components.hpp"

#include <glm/vec3.hpp>

#include "GameObject.hpp"

class TransformComponent : public Component
{
public:
  glm::vec3 position = {0, 0, 0},  rotation = {0, 0, 0}, scale = {1, 1, 1};
};