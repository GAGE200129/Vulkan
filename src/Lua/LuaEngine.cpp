#include "LuaEngine.hpp"

#include <lua.hpp>
#include <cstring>

#include "ECS/Components.hpp"

// int luaAddTransform(lua_State *L)
// {
//   lua_getglobal(L, "E");
//   Entity *e = (Entity *)lua_touserdata(L, -1);
//   e->addComponent<TransformComponent>();

//   lua_pop(L, -1);
//   return 0;
// }

int luaAddComponent(lua_State *L)
{
  lua_getglobal(L, "E");
  entt::entity* e = (entt::entity*)lua_touserdata(L, -1);
  lua_getglobal(L, "ECS");
  entt::registry* registry = (entt::registry*)lua_touserdata(L, -1);

  const char* componentName = lua_tostring(L, 1);
  if(componentName == nullptr)
    throw std::runtime_error("Component name is null");
  
  if(std::strcmp(componentName, "Transform") == 0)
  {
    glm::vec3 pos = {0, 0, 0}, rot = {0, 0, 0} ,scale = {1, 1, 1};
    int paramIndex = 2;
    pos.x = lua_tonumber(L, paramIndex++);
    pos.y = lua_tonumber(L, paramIndex++);
    pos.z = lua_tonumber(L, paramIndex++);

    scale.x = lua_tonumber(L, paramIndex++);
    scale.y = lua_tonumber(L, paramIndex++);
    scale.z = lua_tonumber(L, paramIndex++);

    rot.x = lua_tonumber(L, paramIndex++);
    rot.y = lua_tonumber(L, paramIndex++);
    rot.z = lua_tonumber(L, paramIndex++);

    registry->emplace<TransformComponent>(*e, pos, scale, rot);
  }


  lua_pop(L, -2);
  return 0;
}

void LuaEngine::doTemplate(const std::string& file, float x, float y, float z)
{
  entt::entity entity = mEnTTRegistry.create();
  lua_State *L = luaL_newstate();

  lua_pushlightuserdata(L, &entity);
  lua_setglobal(L, "E");
  lua_pushlightuserdata(L, &mEnTTRegistry);
  lua_setglobal(L, "ECS");

  lua_pushnumber(L, x);
  lua_setglobal(L, "X");

  lua_pushnumber(L, y);
  lua_setglobal(L, "Y");

  lua_pushnumber(L, z);
  lua_setglobal(L, "Z");

  lua_pushcfunction(L, luaAddComponent);
  lua_setglobal(L, "addComponent");

  luaL_dofile(L, file.c_str());

  lua_close(L);
}

void LuaEngine::doTemplate(const std::string &file)
{
  doTemplate(file, 0, 0, 0);
}