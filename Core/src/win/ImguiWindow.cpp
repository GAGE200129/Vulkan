#include "ImguiWindow.hpp"

#include "Exception.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl2.h>
#include <imgui/imgui_impl_glfw.h>
#include <Core/src/log/Log.hpp>

#include <GLFW/glfw3.h>

namespace gage::win
{
    ImguiWindow::ImguiWindow()
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        p_window = glfwCreateWindow(320, 240, "ImGui", nullptr, nullptr);
        if(!p_window)
        {
            logger.error();
            throw WindowException{ "Failed to create window !" };
        }
        glfwMakeContextCurrent(p_window);
        glfwSwapInterval(0);
        // Setup Dear ImGui context
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.IniFilename = "res/imgui.ini";
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL2_Init();
        ImGui_ImplGlfw_InitForOpenGL(p_window, true);

    }
    ImguiWindow::~ImguiWindow()
    {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(p_window);
    }

    void ImguiWindow::clear()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();
    }
    void ImguiWindow::end_frame()
    {
         // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(p_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        

        
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());   
        glfwSwapBuffers(p_window);
    }
}