#include "pch.hpp"
#include "ScriptComponent.hpp"

void ScriptComponent::init()
{
  L = luaL_newstate();
  luaL_openlibs(L);
  int ret = luaL_dofile(L, mFilePath.c_str());
  if(ret != 0)
  {
    throw std::runtime_error("Can't load script file: " + mFilePath);
  }
  
}

void ScriptComponent::update(float delta)
{
  lua_getglobal(L, "update");
  if(!lua_isfunction(L, -1))
    throw std::runtime_error("Can't find update function !");
  lua_pushnumber(L, delta);
  lua_call(L, 1, 0);
}

void ScriptComponent::shutdown() noexcept
{
  lua_close(L);
}