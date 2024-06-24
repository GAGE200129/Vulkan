#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/gfx/Exception.hpp>

#include <thread>

using namespace gage;
using namespace std::chrono_literals;

int main()
{
    log::init();
    win::init();

    try
    {
        std::shared_ptr<win::Window> window, window2;
        window = std::make_shared<win::Window>(640, 480, "Hello world");
        window2 = std::make_shared<win::Window>(640, 480, "Hello world2");

        auto& graphics = window->get_graphics();
        auto& graphics2 = window->get_graphics();
        while (!window->is_closing() && !window2->is_closing())
        {

            graphics.clear(0.5f, 0.0f, 0.0f);
            graphics.draw_test_triangle();
            graphics.end_frame();

            graphics2.clear(0.0f, 0.5f, 0.0f);

            graphics2.end_frame();
            std::this_thread::sleep_for(100ms);
            win::update();
        }
    }
    catch (gfx::GraphicsException &e)
    {
        logger.error("Graphics exception caught: " + std::string(e.what()));
    }

    win::shutdown();

    return 0;
}