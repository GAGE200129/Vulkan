#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/gfx/draw/Box.hpp>
#include <Core/src/gfx/bind/IBindable.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/utils/Camera.hpp>

#include <thread>

#include <Core/ThirdParty/imgui/imgui.h>

using namespace gage;
using namespace std::chrono_literals;

int main()
{
    try
    {
        log::init();
        win::init();

        {
            int resolutions[] = {
                1600, 900
            };

            win::Window window(resolutions[0], resolutions[1], "Hello world");
            win::ImguiWindow imgui_window{};
            utils::Camera camera{};

            auto &graphics = window.get_graphics();

            graphics.set_perspective(resolutions[0], resolutions[1], 70.0f, 0.1f, 1000.0f);

            std::vector<std::unique_ptr<gfx::draw::Box>> boxes;

            for (int i = 0; i < 300; i++)
            {
                boxes.push_back(std::make_unique<gfx::draw::Box>(graphics));
            }

            while (!window.is_closing())
            {
                graphics.set_view(camera.get_view());
                graphics.clear();
                for (auto &box : boxes)
                {
                    box->update(0.016f);
                    box->draw(graphics);
                }
                graphics.end_frame();

                imgui_window.clear();
                ImGui::ShowDemoWindow();
                
                if(ImGui::Begin("Window system"))
                {
                    const char* window_modes[] =
                    {
                        "Windowed",
                        "Fullscreen borderless",
                        "Fullscreen exclusive"
                    };
                    static int selected_window_mode = 0;
                    if (ImGui::BeginListBox("mode"))
                    {
                        for (int n = 0; n < IM_ARRAYSIZE(window_modes); ++n) {
                            const bool is_selected = (selected_window_mode == n);
                            if (ImGui::Selectable(window_modes[n], is_selected)) { selected_window_mode = n; }
                            if (is_selected) { ImGui::SetItemDefaultFocus(); }
                        }
                        ImGui::EndListBox();
                    }
                    ImGui::InputInt2("Resoltuion", resolutions);

                    if(ImGui::Button("Apply"))
                    {
                        window.resize((win::WindowMode)selected_window_mode, resolutions[0], resolutions[1]);
                        graphics.set_perspective(resolutions[0], resolutions[1], 70.0f, 0.1f, 1000.0f);
                    }
                }
                ImGui::End();

                if(ImGui::Begin("Camera"))
                {
                    ImGui::DragFloat3("position", &camera.get_position().x, 0.1f);
                    ImGui::DragFloat3("rotation", &camera.get_rotation().x, 0.1f);
                }
                ImGui::End();
                imgui_window.end_frame();
                win::update();

                std::this_thread::sleep_for(11.11111ms);
            }

            graphics.wait();

            for (auto &box : boxes)
            {
                box->destroy(graphics);
            }
        }
        win::shutdown();
    }
    catch (gfx::GraphicsException &e)
    {
        logger.error("Graphics exception caught: " + std::string(e.what()));
    }
    catch (...)
    {
        logger.info("Unknown exception caught.");
    }

    return 0;
}