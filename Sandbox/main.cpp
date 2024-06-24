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
        window = std::make_shared<win::Window>(320, 240, "Hello world");
        window2 = std::make_shared<win::Window>(320, 240, "Hello world2");
        while (!window->is_closing() && !window2->is_closing())
        {
            window->get_graphics().end_frame();
            window2->get_graphics().end_frame();
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