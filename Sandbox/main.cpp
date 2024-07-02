#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/gfx/draw/Box.hpp>
#include <Core/src/gfx/bind/IBindable.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/draw/StaticModel.hpp>

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
            auto &graphics = window.get_graphics();
            win::ImguiWindow imgui_window{graphics};

            std::vector<std::unique_ptr<gfx::draw::Box>> boxes;
            for (int i = 0; i < 100; i++)
            {
                boxes.push_back(std::make_unique<gfx::draw::Box>(graphics));
            }


            gfx::data::Camera camera{};
            while (!window.is_closing())
            {

                static constexpr int64_t NS_PER_FRAME = (1.0 / 75.0) * 1000000000;
                auto start = std::chrono::high_resolution_clock::now();
                graphics.clear(camera);
                graphics.bind_default_pipeline();
                for (auto &box : boxes)
                {
                    box->update(0.016f);
                    box->draw(graphics);
                }
                // model->draw(graphics);
                graphics.end_frame();

                imgui_window.clear();
                imgui_window.draw(camera, window);
                imgui_window.end_frame();
                win::update();
                auto finish = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(start - finish).count();

                int64_t delay = duration + NS_PER_FRAME;
                if (delay > 0)
                    std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
            }

            graphics.wait();
            boxes.clear();
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