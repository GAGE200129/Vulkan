#include "pch.hpp"
#include "ScriptComponent.hpp"

#include "Vulkan/VulkanEngine.hpp"

#include "Input.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
#include <typeindex>

#include "TransformComponent.hpp"

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

static int luaMouseSetLock(lua_State *L)
{
  bool v = lua_toboolean(L, 1);

  if (v)
    Input::lockCursor();
  else
    Input::unlockCursor();
  return 0;
}



static int luaGameObjectGetComponent(lua_State* L)
{
  static std::map<std::string, std::function<Component*(GameObject* go)>> sComponentMap = 
  {
    {"transform", [](GameObject* go){ return go->getRequiredComponent<TransformComponent>(); }}
  };

  GameObject* go = (GameObject*)lua_touserdata(L, 1);
  std::string name = lua_tostring(L, 2);
  std::transform(name.begin(), name.end(), name.begin(),
    [](unsigned char c){ return std::tolower(c); });

  lua_pushlightuserdata(L, sComponentMap.at(name)(go));
  return 1;
}

static int luaTransformSetPosition(lua_State* L)
{
  TransformComponent* transform = (TransformComponent*)lua_touserdata(L, 1);
  transform->position = {lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)};
  return 0;
}

std::map<std::string, lua_CFunction> ScriptComponent::sFunctionMaps;

void ScriptComponent::registerLuaScript(const std::string &name, lua_CFunction function)
{
  sFunctionMaps[name] = function;
}

void ScriptComponent::init()
{
  L = luaL_newstate();
  luaL_openlibs(L);
  registerLuaScript("vk_camera_update_params", luaUpdateCameraParams);
  registerLuaScript("input_key_is_down", luaIsKeyDown);
  registerLuaScript("input_key_is_down_once", luaIsKeyDownOnce);
  registerLuaScript("input_mouse_get_delta", luaGetMouseDelta);
  registerLuaScript("input_mouse_set_lock", luaMouseSetLock);
  registerLuaScript("vec3_rotate", luaVec3Rotate);
  registerLuaScript("vec3_normalize", luaVec3Normalize);
  registerLuaScript("gameobject_get_component", luaGameObjectGetComponent);
  registerLuaScript("transform_set_position", luaTransformSetPosition);

  for (const auto &[name, function] : sFunctionMaps)
  {
    lua_register(L, name.c_str(), function);
  }

  Input::registerScriptKeys(L);

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