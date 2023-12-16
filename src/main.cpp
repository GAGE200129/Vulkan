#include "pch.hpp"

#include "Vulkan/VulkanTexture.hpp"

#include "ECS/Components.hpp"

#include "ECS/GameObject.hpp"
#include "ECS/ModelComponent.hpp"
#include "ECS/ScriptComponent.hpp"
#include "ECS/AnimatedModelComponent.hpp"
#include "ECS/AnimatorComponent.hpp"
#include "ECS/RigidBodyComponent.hpp"
#include "ECS/BoxColliderComponent.hpp"
#include "ECS/CharacterControllerComponent.hpp"
#include "Bullet/BulletEngine.hpp"

#include "Input.hpp"

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

  mWindow = glfwCreateWindow(1280, 720, "Vulkan", nullptr, nullptr);
  glfwSetFramebufferSizeCallback(mWindow, windowResizeFn);
  BulletEngine::init();
  VulkanEngine::init(mWindow);
  Input::init(mWindow);

  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      GameObject &go = GameObject::addGameObject("Testing");
      TransformComponent *c = go.addComponent<TransformComponent>();
      AnimatorComponent *animator = go.addComponent<AnimatorComponent>();
      go.addComponent<AnimatedModelComponent>("res/models/box.glb");
      go.addComponent<BoxColliderComponent>(glm::vec3{0.0f, 4.0f, 0.0f}, glm::vec3{1.0f, 3.0f, 1.0f});
      go.addComponent<RigidBodyComponent>(100.1f);
      c->position.x = i;
      c->position.z = j;
    }
  }

  GameObject &go1 = GameObject::addGameObject("Testing1");
  go1.addComponent<TransformComponent>();
  go1.addComponent<ModelComponent>("res/models/box_textured.glb");
  go1.addComponent<CharacterControllerComponent>();
  go1.addComponent<ScriptComponent>("res/scripts/testing.lua");

  GameObject::globalInit();

  double lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(mWindow))
  {
    double current = glfwGetTime();
    double elapsed = current - lastTime;
    lastTime = current;
    glfwPollEvents();

    BulletEngine::update(elapsed);
    GameObject::globalUpdate(elapsed);

    Input::update();
    VulkanEngine::render();
  }
  VulkanEngine::joint();

  GameObject::globalShutdown();
  ModelComponent::clearCache();
  AnimatedModelComponent::clearCache();
  VulkanEngine::cleanup();
  BulletEngine::cleanup();
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