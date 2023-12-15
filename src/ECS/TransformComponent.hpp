#pragma once

#include "Components.hpp"
#include "GameObject.hpp"

class TransformComponent : public Component
{
public:
  glm::mat4 build() const noexcept
  {
    glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 rotateM = glm::mat4(rotation);
    glm::mat4 positionM = glm::translate(glm::mat4(1.0f), position);

    return positionM * rotateM * scaleM;
  }

  static void registerLuaScript(lua_State* L);
public:
  glm::vec3 position = {0, 0, 0}, scale = {1, 1, 1};
  glm::quat rotation = glm::quat(1, 0, 0, 0);
};
