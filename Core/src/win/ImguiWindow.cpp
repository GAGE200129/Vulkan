#include <pch.hpp>
#include "ImguiWindow.hpp"

#include "Exception.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/SSAO.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/data/Swapchain.hpp>
#include <Core/src/gfx/data/AmbientLight.hpp>
#include <Core/src/gfx/data/DirectionalLight.hpp>

#include <Core/src/scene/SceneGraph.hpp>
#include <Core/src/mem.hpp>


#include "Window.hpp"

namespace gage::win
{
    ImguiWindow::ImguiWindow(gfx::Graphics &gfx) : gfx(gfx)
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
        //create_viewport(gfx);
        
    }
    ImguiWindow::~ImguiWindow()
    {
        //destroy_viewport();

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
    }

    void ImguiWindow::draw(gfx::data::Camera &camera, Window &window, scene::SceneGraph& scene)
    {
        static int resolutions[] = {1600, 900};
        ImGui::ShowDemoWindow();

        if (ImGui::Begin("Rendering system"))
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

            static float resolution_scale = 1.0f;
            ImGui::DragFloat("Resolution scale", &gfx.draw_extent_scale, 0.01f, 0.1f, 1.0f);
            

            if (ImGui::Button("Apply"))
            {
                window.resize((win::WindowMode)selected_window_mode, resolutions[0], resolutions[1]);
                gfx.resize(resolutions[0], resolutions[1]);
            }
            ImGui::Separator();
             static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            ImGui::Text("Frame time: %f ms", stats.frame_time);
            ImGui::Text("Mem allocated: %lu bytes", gage::get_allocated_bytes());
            
        }
        ImGui::End();

        if (ImGui::Begin("Camera"))
        {
            ImGui::DragFloat3("position", &camera.get_position().x, 0.1f);
            ImGui::DragFloat3("rotation", &camera.get_rotation().x, 0.1f);
        }
        ImGui::End();


        if (ImGui::Begin("Lightning"))
        {
            static int resolution = 1024;
            static float distances[] = {10.0f, 20.0f, 50.0f};
            static float ssao_radius = 0.5f;
            static float ssao_bias = 0.025f;
            auto& ubo = gfx.global_uniform;

            ImGui::ColorEdit3("Ambient: color", &ubo.ambient_light_color.x);
            ImGui::DragFloat("Ambient: intensity", &ubo.ambient_light_intensity, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Ambient: sky scale", &gfx.final_ambient->fs_ps.fbm_scale, 0.01f, 0.0f, 10.01f);
            ImGui::DragFloat("Ambient: sky factor", &gfx.final_ambient->fs_ps.fbm_factor, 0.01f, 0.0f, 10.01f);
            ImGui::DragFloat("Ambient: sky height", &gfx.final_ambient->fs_ps.height, 0.01f, 0.0f, 10.01f);
            ImGui::DragFloat("Ambient: SSAO radius", &gfx.ssao->fs_ps.radius, 0.01f, 0.001f, 1.0f);
            ImGui::DragFloat("Ambient: SSAO bias", &gfx.ssao->fs_ps.bias, 0.001f, 0.001f, 1.0f);
    
            ImGui::DragFloat("Ambient: Fog begin", &ubo.ambient_fog_begin, 0.1f, 10.0f, 10000.0f);
            ImGui::DragFloat("Ambient: Fog end", &ubo.ambient_fog_end, 0.1f, 10.0f, 10000.0f);
            ImGui::Separator();
            if(ImGui::DragFloat3("Directional: direction", &ubo.directional_light_direction.x, 0.01f, -1, 1))
            {
                ubo.directional_light_direction = glm::normalize(ubo.directional_light_direction);
            }
            ImGui::ColorEdit3("Directional: color", &ubo.directional_light_color.x);
            if(ImGui::DragInt("Directional: Shadow map resolution", &resolution, 1.0f, 512, 4096))
            {
                gfx.resize_shadow_map(resolution);
            }
            bool distances_dirty = ImGui::DragFloat("Directional: Shadow map cascade distance 1", &distances[0], 0.1f, 0.1f, 2048.0f);
            distances_dirty |= ImGui::DragFloat("Directional: Shadow map cascade distance 2", &distances[1], 0.1f, 0.1f, 2048.0f);
            distances_dirty |= ImGui::DragFloat("Directional: Shadow map cascade distance 3", &distances[2], 0.1f, 0.1f, 2048.0f);
            if(distances_dirty)
            {
                ubo.directional_light_cascade_planes[0].x = distances[0];
                ubo.directional_light_cascade_planes[1].x = distances[1];
                ubo.directional_light_cascade_planes[2].x = distances[2];
            }


            ImGui::Separator();
        }
        ImGui::End();


        scene.render_imgui();
    }
}