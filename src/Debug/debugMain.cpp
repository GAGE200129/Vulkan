#include "pch.hpp"
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl2.h>
#include <backends/imgui_impl_glfw.h>

#include "DebugCamera.hpp"
#include "EntityInspector.hpp"
#include "ECS/GameObject.hpp"

extern GLFWwindow *gMainWindow;
bool gDebugPaused = false;

static void focusFn(GLFWwindow *window, int focused)
{
  gDebugPaused = focused;
}

void debugMain()
{
  GLFWwindow *pWindow;

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  pWindow = glfwCreateWindow(1024, 768, "Debugger", nullptr, nullptr);
  glfwMakeContextCurrent(pWindow);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
  ImGui_ImplOpenGL2_Init();

  DebugCamera camera(pWindow);

  double lastTime = glfwGetTime();
  double lag = 0;
  constexpr double MS_PER_TICK = 1.0 / 60.0;
  glfwSetWindowFocusCallback(pWindow, focusFn);
  while (!glfwWindowShouldClose(gMainWindow))
  {
    double current = glfwGetTime();
    double elapsed = current - lastTime;
    lastTime = current;
    lag += elapsed;

    while (lag >= MS_PER_TICK)
    {
      glfwPollEvents();
      camera.update(elapsed);
      lag -= MS_PER_TICK;
    }

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
    EntityInspector::renderImGui();

    ImGui::ShowDemoWindow();

    ImGui::Render();
      int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::mat4 proj = camera.getPerspective();
    glm::mat4 view = camera.getView();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(proj));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(view));

    //Render all gameobjects, wireframe mode
    GameObject::globalDebugRender();

    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(100, 0, 0);

    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 100, 0);

    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 100);
    glEnd();

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(pWindow);
    glfwSwapInterval(1);
  }
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(pWindow);
}