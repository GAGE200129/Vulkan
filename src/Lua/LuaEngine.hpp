#pragma once

#include <vector>
#include <string>
#include "entt/entt.hpp"

class LuaEngine
{
public:
  LuaEngine(entt::registry& registry) : mEnTTRegistry(registry) {}

  void doTemplate(const std::string& file);
  void doTemplate(const std::string& file, float x, float y, float z);
private:
  entt::registry& mEnTTRegistry;
};