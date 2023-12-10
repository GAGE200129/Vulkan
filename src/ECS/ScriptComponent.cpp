#include "pch.hpp"
#include "ScriptComponent.hpp"

#include "Vulkan/VulkanEngine.hpp"

#include "Input.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

int luaUpdateCameraParams(lua_State *L)
{
  auto &camera = VulkanEngine::getCamera();
  camera.position.x = lua_tonumber(L, 1);
  camera.position.y = lua_tonumber(L, 2);
  camera.position.z = lua_tonumber(L, 3);

  camera.pitch = lua_tonumber(L, 4);
  camera.yaw = lua_tonumber(L, 5);

  camera.fov = lua_tonumber(L, 6);
  camera.nearPlane = lua_tonumber(L, 7);
  camera.farPlane = lua_tonumber(L, 8);

  return 0;
}

int luaIsKeyDown(lua_State *L)
{
  int key = lua_tointeger(L, 1);

  lua_pushboolean(L, Input::isKeyDown(key));
  return 1;
}

int luaIsKeyDownOnce(lua_State *L)
{
  int key = lua_tointeger(L, 1);

  lua_pushboolean(L, Input::isKeyDownOnce(key));
  return 1;
}

int luaGetMouseDelta(lua_State *L)
{
  lua_pushnumber(L, Input::getDx());
  lua_pushnumber(L, Input::getDy());
  return 2;
}

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

static int luaVec3Normalize(lua_State* L)
{
  glm::vec3 v;
  v.x = lua_tonumber(L, 1);
  v.y = lua_tonumber(L, 2);
  v.z = lua_tonumber(L, 3);


  if(glm::length2(v) != 0.0f)
  {
    v = glm::normalize(v);
  }

  lua_pushnumber(L, v.x);
  lua_pushnumber(L, v.y);
  lua_pushnumber(L, v.z);

  return 3;
}

static int luaMouseSetLock(lua_State* L)
{
  bool v = lua_toboolean(L, 1);

  if(v)
    Input::lockCursor();
  else
    Input::unlockCursor();
  return 0;
}


void ScriptComponent::init()
{
  L = luaL_newstate();
  lua_register(L, "vk_camera_update_params", luaUpdateCameraParams);
  lua_register(L, "input_key_is_down", luaIsKeyDown);
  lua_register(L, "input_key_is_down_once", luaIsKeyDownOnce);
  lua_register(L, "input_mouse_get_delta", luaGetMouseDelta);
  lua_register(L, "input_mouse_set_lock", luaMouseSetLock);
  lua_register(L, "vec3_rotate", luaVec3Rotate);
  lua_register(L, "vec3_normalize", luaVec3Normalize);

  Input::registerScriptKeys(L);

  int ret = luaL_dofile(L, mFilePath.c_str());
  if (ret != 0)
  {
    throw std::runtime_error("Can't load script file: " + mFilePath);
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