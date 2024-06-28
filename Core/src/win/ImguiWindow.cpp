#include "ImguiWindow.hpp"

#include "Exception.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <Core/src/log/Log.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/utils/Camera.hpp>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Window.hpp"

namespace gage::win
{
    ImguiWindow::ImguiWindow(gfx::Graphics &gfx)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        p_window = glfwCreateWindow(800, 600, "ImGui", nullptr, nullptr);
        if (!p_window)
        {
            logger.error();
            throw WindowException{"Failed to create window !"};
        }
        glfwMakeContextCurrent(p_window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        glfwSwapInterval(0);
        // Setup Dear ImGui context
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.IniFilename = "res/imgui.ini";
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init();
        ImGui_ImplGlfw_InitForOpenGL(p_window, true);
    }
    ImguiWindow::~ImguiWindow()
    {

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(p_window);
    }

    void ImguiWindow::clear()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
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

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(p_window);

        glDeleteTextures(1, &gfx_color_texture);
        glDeleteMemoryObjectsEXT(1, &gfx_color_texture_mem);

        glDeleteTextures(1, &gfx_depth_texture);
        glDeleteMemoryObjectsEXT(1, &gfx_depth_texture_mem);
    }

    void ImguiWindow::draw(utils::Camera &camera, Window &window)
    {

        const auto [color_fd, color_size] = window.get_graphics().get_color_image();
        const auto [depth_fd, depth_size] = window.get_graphics().get_depth_image();
        VkExtent2D extent = window.get_graphics().get_scaled_draw_extent();

        glCreateMemoryObjectsEXT(1, &gfx_color_texture_mem);
        glImportMemoryFdEXT(gfx_color_texture_mem, color_size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, color_fd);
        glCreateTextures(GL_TEXTURE_2D, 1, &gfx_color_texture);
        glTextureStorageMem2DEXT(gfx_color_texture, 1, GL_SRGB8, extent.width, extent.height, gfx_color_texture_mem, 0);

        glCreateMemoryObjectsEXT(1, &gfx_depth_texture_mem);
        glImportMemoryFdEXT(gfx_depth_texture_mem, depth_size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, depth_fd);
        glCreateTextures(GL_TEXTURE_2D, 1, &gfx_depth_texture);
        glTextureStorageMem2DEXT(gfx_depth_texture, 1, GL_DEPTH_COMPONENT32, extent.width, extent.height, gfx_depth_texture_mem, 0);

        static int resolutions[] = {1600, 900};
        ImGui::ShowDemoWindow();

        if (ImGui::Begin("Window system"))
        {
            const char *window_modes[] =
                {
                    "Windowed",
                    "Fullscreen borderless",
                    "Fullscreen exclusive"};
            static int selected_window_mode = 0;
            if (ImGui::BeginListBox("mode"))
            {
                for (int n = 0; n < IM_ARRAYSIZE(window_modes); ++n)
                {
                    const bool is_selected = (selected_window_mode == n);
                    if (ImGui::Selectable(window_modes[n], is_selected))
                    {
                        selected_window_mode = n;
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndListBox();
            }
            ImGui::InputInt2("Resoltuion", resolutions);

            static int resolution_scale = 100;
            ImGui::DragInt("Resolution scale", &resolution_scale, 1, 20, 200);

            if (ImGui::Button("Apply"))
            {
                window.resize((win::WindowMode)selected_window_mode, resolutions[0], resolutions[1], (float)resolution_scale / 100.0f);
                window.get_graphics().set_perspective(resolutions[0], resolutions[1], 70.0f, 0.1f, 1000.0f);
            }
        }
        ImGui::End();

        if (ImGui::Begin("Camera"))
        {
            ImGui::DragFloat3("position", &camera.get_position().x, 0.1f);
            ImGui::DragFloat3("rotation", &camera.get_rotation().x, 0.1f);
        }
        ImGui::End();



        if (ImGui::Begin("Viewport-Color"))
        {
            ImGui::Image((ImTextureID)gfx_color_texture, ImGui::GetContentRegionMax());
        }
        ImGui::End();
        if (ImGui::Begin("Viewport-Depth"))
        {
            ImGui::Image((ImTextureID)gfx_depth_texture, ImGui::GetContentRegionMax());
        }
        ImGui::End();
    }
}