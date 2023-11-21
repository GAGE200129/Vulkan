#pragma once

#include <glm/vec3.hpp>
#include "Model/Model.hpp"

struct TransformComponent
{
  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 rotation_euler;
};

struct ModelComponent
{
  Model* pModel;
  
};

