#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/gfx/Exception.hpp>

using namespace gage;
using namespace std::chrono_literals;

int main()
{
    try
    {
        log::init();
        win::init();

        std::unique_ptr<win::Window> window = std::make_unique<win::Window>(640, 480, "Hello world");
        std::unique_ptr<win::Window> window2 = std::make_unique<win::Window>(640, 480, "Hello world2");

        auto &graphics = window->get_graphics();
        auto &graphics2 = window2->get_graphics();

        while (!window->is_closing() && !window2->is_closing())
        {

            graphics.clear(0.5f, 0.0f, 0.0f);

            graphics.end_frame();

            graphics2.clear(0.0f, 0.5f, 0.0f);

            graphics2.end_frame();
            win::update();
        }
        window.reset();
        window2.reset();
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