#include "pch.hpp"
#include "TransformComponent.hpp"

#include "RigidBodyComponent.hpp"
#include "CharacterControllerComponent.hpp"

#include <imgui.h>

void TransformComponent::registerLuaScript(lua_State *L)
{
  auto luaTransformSetPosition = [](lua_State *L) -> int
  {
    TransformComponent *transform = (TransformComponent *)lua_touserdata(L, 1);
    transform->position = {lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)};
    return 0;
  };

  auto luaTransformGetPosition = [](lua_State *L) -> int
  {
    TransformComponent *transform = (TransformComponent *)lua_touserdata(L, 1);
    lua_pushnumber(L, transform->position.x);
    lua_pushnumber(L, transform->position.y);
    lua_pushnumber(L, transform->position.z);
    return 3;
  };
  lua_register(L, "transform_set_position", luaTransformSetPosition);
  lua_register(L, "transform_get_position", luaTransformGetPosition);
}

void TransformComponent::renderImGui() noexcept
{
  ImGui::Text("Transform component");
  bool positionUpdated = false, rotationUpdated = false;
  positionUpdated |= ImGui::DragFloat3("Position", &position.x, 0.1f);
  rotationUpdated |= ImGui::DragFloat4("Rotation", &rotation.x, 0.01f, -1.0f, 1.0f);

  auto updateRigidBodyTransform = [this](btRigidBody* pBody)
  {
    btTransform &t = pBody->getWorldTransform();
    btVector3 origin = t.getOrigin();
    btQuaternion q = t.getRotation();
    origin.setValue(position.x, position.y, position.z);
    q.setValue(rotation.x, rotation.y, rotation.z, rotation.w);
    t.setOrigin(origin);
    t.setRotation(q);
    pBody->activate();
  };

  if (rotationUpdated)
  {
    rotation = glm::normalize(rotation);
  }

  if (rotationUpdated || positionUpdated)
  {
    RigidBodyComponent *pRigidBody = mGameObject->getComponent<RigidBodyComponent>();
    CharacterControllerComponent* pCharacterController = mGameObject->getComponent<CharacterControllerComponent>();
    if (pRigidBody)
    {
      updateRigidBodyTransform(pRigidBody->mBody);
    }
    if(pCharacterController)
    {
      updateRigidBodyTransform(pCharacterController->mBody);
    }
  }
}