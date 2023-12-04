

#include <iostream>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Lua/LuaEngine.hpp"
#include "Vulkan/VulkanTexture.hpp"

#include "ECS/Components.hpp"

#include "Vulkan/VulkanTexture.hpp"

#include "ECS/GameObject.hpp"
#include "ECS/ModelComponent.hpp"

void windowResizeFn(GLFWwindow *window, int width, int height) noexcept
{
  VulkanEngine::onWindowResize(width, height);
}

void run()
{
  GLFWwindow *mWindow;
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  mWindow = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
  glfwSetFramebufferSizeCallback(mWindow, windowResizeFn);

  VulkanEngine::init(mWindow);

  GameObject &go = GameObject::addGameObject("Testing");
  go.addComponent<ModelComponent>("res/models/adamHead/adamHead.gltf");
  go.addComponent<TransformComponent>();

  GameObject &go1 = GameObject::addGameObject("Testing1");
  go1.addComponent<ModelComponent>("res/models/box.glb");
  TransformComponent *c = go1.addComponent<TransformComponent>();
  c->position.x += 3;

  GameObject::globalInit();
  while (!glfwWindowShouldClose(mWindow))
  {
    GameObject::globalUpdate(0.16f);
    if (VulkanEngine::prepare())
    {
      GameObject::globalRender();
      VulkanEngine::submit();
    }

    glfwPollEvents();
  }
  VulkanEngine::joint();

  GameObject::globalShutdown();
  ModelComponent::clearCache();
  VulkanEngine::cleanup();
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

int main()
{
  try
  {
    run();
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}