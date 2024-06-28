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
            win::Window window(800, 600, "Hello world");
            utils::Camera camera{};
            auto &graphics = window.get_graphics();
            win::ImguiWindow imgui_window{graphics};

            graphics.set_perspective(800, 600, 70.0f, 0.1f, 1000.0f);

            std::vector<std::unique_ptr<gfx::draw::Box>> boxes;

            for (int i = 0; i < 300; i++)
            {
                boxes.push_back(std::make_unique<gfx::draw::Box>(graphics));
            }

            //uint32_t color_texure = graphics.get_color_image(); 

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
                imgui_window.draw(camera, window);
                imgui_window.end_frame();
                win::update();
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