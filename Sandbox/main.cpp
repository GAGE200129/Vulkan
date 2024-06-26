#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/gfx/draw/Box.hpp>
#include <Core/src/gfx/bind/IBindable.hpp>

#include <thread>

using namespace gage;
using namespace std::chrono_literals;

int main()
{
    try
    {
        log::init();
        win::init();

        {
            win::Window window(640, 480, "Hello world");
            win::Window window2(640, 480, "Hello world2");

            auto &graphics = window.get_graphics();
            auto &graphics2 = window2.get_graphics();

            graphics.set_perspective(640, 480, 70.0f, 0.1f, 1000.0f);
            graphics2.set_perspective(640, 480, 70.0f, 0.1f, 1000.0f);

            gfx::draw::Box box(graphics);
            gfx::draw::Box box2(graphics2);

            while (!window.is_closing())
            {
                box2.update(0.56f);

                graphics.clear();
                box.draw(graphics);
                graphics.end_frame();

                graphics2.clear();
                box2.draw(graphics2);
                graphics2.end_frame();
                win::update();

                std::this_thread::sleep_for(10ms);
            }

            graphics.wait();
            graphics2.wait();

            box.destroy(graphics);
            box2.destroy(graphics2);
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