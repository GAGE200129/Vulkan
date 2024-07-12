#include <pch.hpp>
#include "ImguiWindow.hpp"

#include "Exception.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <Core/src/log/Log.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/data/Swapchain.hpp>
#include <Core/src/gfx/data/DefaultPipeline.hpp>


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

    void ImguiWindow::draw(gfx::data::Camera &camera, Window &window)
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

            static int resolution_scale = 100;
            ImGui::DragInt("Resolution scale", &resolution_scale, 1, 20, 200);

            if (ImGui::Button("Apply"))
            {
                window.resize((win::WindowMode)selected_window_mode, resolutions[0], resolutions[1], (float)resolution_scale / 100.0f);
            }
            ImGui::Separator();
             static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            ImGui::Text("Frame time: %f ms", stats.frame_time);
            
        }
        ImGui::End();

        if (ImGui::Begin("Camera"))
        {
            ImGui::DragFloat3("position", &camera.get_position().x, 0.1f);
            ImGui::DragFloat3("rotation", &camera.get_rotation().x, 0.1f);
        }
        ImGui::End();

        //if (ImGui::Begin("Viewport-Color"))
        //{
        //    ImGui::Image((ImTextureID)gfx_color_texture, ImGui::GetContentRegionMax());
        //}
        //ImGui::End();
        //if (ImGui::Begin("Viewport-Depth"))
        //{
        //    ImGui::Image((ImTextureID)gfx_depth_texture, ImGui::GetContentRegionMax());
        //}
        //ImGui::End();

        if (ImGui::Begin("Lightning"))
        {
            static int resolution = 1024;
            static float distance = 50.0f;
            auto& ubo = window.get_graphics().get_global_uniform();
            ImGui::Text("Directional light");
            if(ImGui::DragFloat3("Direction", &ubo.directional_light.direction.x, 0.01f, -1, 1))
            {
                ubo.directional_light.direction = glm::normalize(ubo.directional_light.direction);
            }
            ImGui::ColorEdit3("Color", &ubo.directional_light.color.x);
            if(ImGui::DragInt("Shadow map resolution", &resolution, 1.0f, 512, 4096))
            {
                window.get_graphics().resize_shadow_map(resolution);
            }
            if(ImGui::DragFloat("Shadow map distance", &distance, 0.1f, 10.0f, 2048.0f))
            {
                window.get_graphics().set_shadow_distance(distance);
            }

            ImGui::Separator();
        }
        ImGui::End();
    }

   // void ImguiWindow::create_viewport(gfx::Graphics &gfx)
   // {
   //     const auto [color_fd, color_size] = gfx.get_swapchain().get_color_image_external();
   //     const auto [depth_fd, depth_size] = gfx.get_swapchain().get_depth_image_external();
   //     VkExtent2D extent = gfx.get_scaled_draw_extent();
//
   //     glCreateMemoryObjectsEXT(1, &gfx_color_texture_mem);
   //     glImportMemoryFdEXT(gfx_color_texture_mem, color_size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, color_fd);
   //     glCreateTextures(GL_TEXTURE_2D, 1, &gfx_color_texture);
//
   //     GLint color_swizzle[4] = { //Vulkan using BGRA8 not RGBA8
   //         GL_BLUE,
   //         GL_GREEN,
   //         GL_RED,
   //         GL_ALPHA
   //     };
   //     glTextureParameteriv(gfx_color_texture, GL_TEXTURE_SWIZZLE_RGBA, color_swizzle);
   //     glTextureStorageMem2DEXT(gfx_color_texture, 1, GL_RGBA8, extent.width, extent.height, gfx_color_texture_mem, 0);
//
   //     glCreateMemoryObjectsEXT(1, &gfx_depth_texture_mem);
   //     glImportMemoryFdEXT(gfx_depth_texture_mem, depth_size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, depth_fd);
   //     glCreateTextures(GL_TEXTURE_2D, 1, &gfx_depth_texture);
//
   //     GLint swizzle[4] = {
   //         GL_RED,
   //         GL_RED,
   //         GL_RED,
   //         GL_ONE};
   //     glTextureParameteriv(gfx_depth_texture, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
   //     glTextureStorageMem2DEXT(gfx_depth_texture, 1, GL_DEPTH_COMPONENT32, extent.width, extent.height, gfx_depth_texture_mem, 0);
   // }
   // void ImguiWindow::destroy_viewport()
   // {
   //     glDeleteTextures(1, &gfx_color_texture);
   //     glDeleteMemoryObjectsEXT(1, &gfx_color_texture_mem);
//
   //     glDeleteTextures(1, &gfx_depth_texture);
   //     glDeleteMemoryObjectsEXT(1, &gfx_depth_texture_mem);
   // }
}