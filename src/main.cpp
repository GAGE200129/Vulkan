#include "pch.hpp"

#include "Vulkan/VulkanTexture.hpp"

#include "ECS/Components.hpp"

#include "ECS/GameObject.hpp"
#include "ECS/ModelComponent.hpp"
#include "ECS/ScriptComponent.hpp"
#include "ECS/AnimatedModelComponent.hpp"
#include "ECS/AnimatorComponent.hpp"

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

  VulkanEngine::init(mWindow);
  Input::init(mWindow);

  GameObject &camera = GameObject::addGameObject("Camera");
  camera.addComponent<TransformComponent>();
  camera.addComponent<ScriptComponent>("res/scripts/testing.lua");

  GameObject &go = GameObject::addGameObject("Testing");
  AnimatorComponent* animator = go.addComponent<AnimatorComponent>();
  go.addComponent<AnimatedModelComponent>("res/models/box.glb");
  TransformComponent *c = go.addComponent<TransformComponent>();
  

  GameObject &go1 = GameObject::addGameObject("Testing1");
  go1.addComponent<ModelComponent>("res/models/box_textured.glb");
  go1.addComponent<TransformComponent>();
  go1.addComponent<ScriptComponent>("res/scripts/moveTest.lua");

  GameObject::globalInit();

  animator->setCurrentAnimation("Crazy1");

  double lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(mWindow))
  {
    double current = glfwGetTime();
    double elapsed = current - lastTime;
    glfwPollEvents();
    
    GameObject::globalUpdate(elapsed);

    if(Input::isKeyDownOnce(GLFW_KEY_C))
    {
      std::cout << "AKFJAKSF\n";
    }

    Input::update();
    VulkanEngine::render();

    lastTime = current;
  }
  VulkanEngine::joint();

  GameObject::globalShutdown();
  ModelComponent::clearCache();
  AnimatedModelComponent::clearCache();
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