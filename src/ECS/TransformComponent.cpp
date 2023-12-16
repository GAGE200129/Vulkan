#include "pch.hpp"
#include "TransformComponent.hpp"

void TransformComponent::registerLuaScript(lua_State* L)
{
  auto luaTransformSetPosition = [](lua_State* L) -> int
  {
    TransformComponent* transform = (TransformComponent*)lua_touserdata(L, 1);
    transform->position = {lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)};
    return 0;
  };

  auto luaTransformGetPosition = [](lua_State* L) -> int
  {
    TransformComponent* transform = (TransformComponent*)lua_touserdata(L, 1);
    lua_pushnumber(L, transform->position.x);
    lua_pushnumber(L, transform->position.y);
    lua_pushnumber(L, transform->position.z);
    return 3;
  };
  lua_register(L, "transform_set_position", luaTransformSetPosition);
  lua_register(L, "transform_get_position", luaTransformGetPosition);
}