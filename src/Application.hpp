#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Vulkan/VulkanEngine.hpp"
#include "Lua/LuaEngine.hpp"
#include "entt/entt.hpp"
#include "Model/Model.hpp"
#include "Vulkan/VulkanTexture.hpp"


class Application
{
public:
  Application() : mVulkanEngine(mWindow, mEntt), mLuaEngine(mEntt), mModel(mVulkanEngine), mTexture(mVulkanEngine) {}
  void run()
  {
    init();
    mainLoop();
    cleanup();
  }

private:
  void init();
  void mainLoop();
  void cleanup() noexcept;

  static void windowResizeFn(GLFWwindow* window, int width, int height) noexcept;

private:
  GLFWwindow *mWindow;
  entt::registry mEntt;
  VulkanEngine mVulkanEngine;
  LuaEngine mLuaEngine;
  Model mModel;
  VulkanTexture mTexture;
};