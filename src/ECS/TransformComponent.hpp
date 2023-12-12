#pragma once

#include "Components.hpp"
#include "GameObject.hpp"

class TransformComponent : public Component
{
public:
  glm::mat4 build() const noexcept
  {
    glm::mat4 result;
    result = glm::scale(glm::mat4(1.0f), scale);
    result = glm::rotate(result, glm::radians(rotation.x), {1, 0, 0});
    result = glm::rotate(result, glm::radians(rotation.y), {0, 1, 0});
    result = glm::rotate(result, glm::radians(rotation.z), {0, 0, 1});
    result = glm::translate(result, position);

    return result;
  }
public:
  glm::vec3 position = {0, 0, 0},  rotation = {0, 0, 0}, scale = {1, 1, 1};
};
