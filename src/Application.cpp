#include "Application.hpp"

#include "ECS/Components.hpp"

#include "Vulkan/VulkanTexture.hpp"

void Application::windowResizeFn(GLFWwindow *window, int width, int height) noexcept
{
  Application *app = (Application *)glfwGetWindowUserPointer(window);
  app->mVulkanEngine.onWindowResize(width, height);
}


void Application::init()
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  mWindow = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(mWindow, this);
  glfwSetFramebufferSizeCallback(mWindow, windowResizeFn);

  mVulkanEngine.init();


  mLuaEngine.doTemplate("res/entityTemplate/TestObject.lua");
  mLuaEngine.doTemplate("res/entityTemplate/TestObject.lua", 1, 0, 0);
  mLuaEngine.doTemplate("res/entityTemplate/TestObject.lua", 1, 1, 0);


  mModel.init("res/models/box.glb");

  auto e = mEntt.create();
  mEntt.emplace<ModelComponent>(e, &mModel);
  mEntt.emplace<TransformComponent>(e, glm::vec3{0, 0, 0}, glm::vec3{10, 10, 10}, glm::vec3{0, 0, 0});

  auto e1 = mEntt.create();
  mEntt.emplace<ModelComponent>(e1, &mModel);
  mEntt.emplace<TransformComponent>(e1, glm::vec3{3, 0, 0}, glm::vec3{10, 10, 10}, glm::vec3{0, 0, 0});
  
  mTexture.loadFromFile("res/models/adamHead/Assets/Models/PBR/Adam/Textures/Adam_Head_a.jpg");

}
void Application::mainLoop()
{

  while (!glfwWindowShouldClose(mWindow))
  {
    mVulkanEngine.render(mTexture);
    glfwPollEvents();
  }
  mVulkanEngine.joint();
}

void Application::cleanup() noexcept
{
  mTexture.cleanup();
  mModel.cleanup();
  mVulkanEngine.cleanup();
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}