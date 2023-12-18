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

  mWindow = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
  glfwSetFramebufferSizeCallback(mWindow, windowResizeFn);
  BulletEngine::init();
  VulkanEngine::init(mWindow);
  Input::init(mWindow);

  std::vector<AnimatorComponent*> animators;

  for (int i = 5; i < 15; i += 2)
  {
    for (int j = 5; j < 15; j += 2)
    {
      GameObject *go = &GameObject::addGameObject("Testing");
      TransformComponent *c = go->addComponent<TransformComponent>();
      animators.push_back(go->addComponent<AnimatorComponent>());
      go->addComponent<AnimatedModelComponent>("res/models/box.glb");
      go->addComponent<BoxColliderComponent>(glm::vec3{0.0f, 3.0f, 0.0f}, glm::vec3{1.0f, 3.0f, 1.0f});
      go->addComponent<RigidBodyComponent>(1.0f);
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

  for(const auto& animator : animators)
  {
    animator->setCurrentAnimation("Crazy");
  }

  double lastTime = glfwGetTime();
  double lag = 0;
  constexpr double MS_PER_TICK = 1.0 / 60.0;
  while (!glfwWindowShouldClose(mWindow))
  {
    double current = glfwGetTime();
    double elapsed = current - lastTime;
    lastTime = current;
    lag += elapsed;

    while (lag >= MS_PER_TICK)
    {
      glfwPollEvents();
      BulletEngine::update(MS_PER_TICK);
      GameObject::globalUpdate(MS_PER_TICK);
      Input::update();
      lag -= MS_PER_TICK;
    }

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