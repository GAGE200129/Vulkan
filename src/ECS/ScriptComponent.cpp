#include "pch.hpp"
#include "ScriptComponent.hpp"

#include "Vulkan/VulkanEngine.hpp"

#include "Input.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include "TransformComponent.hpp"
#include "CharacterControllerComponent.hpp"
#include "Vulkan/VulkanEngine.hpp"
#include "GameObject.hpp"

int luaVec3Rotate(lua_State *L)
{
  glm::vec3 v, dir;
  float angle;

  int idx = 1;
  v.x = lua_tonumber(L, idx++);
  v.y = lua_tonumber(L, idx++);
  v.z = lua_tonumber(L, idx++);

  angle = lua_tonumber(L, idx++);
  dir.x = lua_tonumber(L, idx++);
  dir.y = lua_tonumber(L, idx++);
  dir.z = lua_tonumber(L, idx++);

  v = glm::rotate(v, glm::radians(angle), dir);

  lua_pushnumber(L, v.x);
  lua_pushnumber(L, v.y);
  lua_pushnumber(L, v.z);

  return 3;
}

static int luaVec3Normalize(lua_State *L)
{
  glm::vec3 v;
  v.x = lua_tonumber(L, 1);
  v.y = lua_tonumber(L, 2);
  v.z = lua_tonumber(L, 3);

  if (glm::length2(v) != 0.0f)
  {
    v = glm::normalize(v);
  }

  lua_pushnumber(L, v.x);
  lua_pushnumber(L, v.y);
  lua_pushnumber(L, v.z);

  return 3;
}

void ScriptComponent::init()
{
  L = luaL_newstate();
  luaL_openlibs(L);

  VulkanEngine::registerLuaScript(L);
  Input::registerLuaScript(L);
  GameObject::registerLuaScript(L);
  TransformComponent::registerLuaScript(L);
  CharacterControllerComponent::registerLuaScript(L);

  lua_register(L, "vec3_rotate", luaVec3Rotate);
  lua_register(L, "vec3_normalize", luaVec3Normalize);

  int ret = luaL_dofile(L, mFilePath.c_str());
  if (ret != 0)
  {
    std::string error = "no error";
    if (lua_isstring(L, lua_gettop(L)))
      error = lua_tostring(L, -1);
    throw std::runtime_error("Can't load script file: " + mFilePath + " | error: " + error);
  }
}

void ScriptComponent::lateInit()
{
  lua_getglobal(L, "init");
  if (lua_isfunction(L, -1))
  {
    lua_pushlightuserdata(L, mGameObject);
    lua_call(L, 1, 0);
  }
}

void ScriptComponent::update(float delta)
{
  lua_getglobal(L, "update");
  if (!lua_isfunction(L, -1))
    throw std::runtime_error("Can't find update function !");
  lua_pushnumber(L, delta);
  lua_call(L, 1, 0);
}

void ScriptComponent::shutdown() noexcept
{
  lua_close(L);
}