#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/gfx/draw/Box.hpp>
#include <Core/src/gfx/bind/IBindable.hpp>
#include <Core/src/utils/FileLoader.hpp>

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
            win::Window window(1600, 900, "Hello world");

            auto &graphics = window.get_graphics();

            graphics.set_perspective(1600, 900, 70.0f, 0.1f, 1000.0f);

            std::vector<std::unique_ptr<gfx::draw::Box>> boxes;

            

            for (int i = 0; i < 100; i++)
            {
                boxes.push_back(std::make_unique<gfx::draw::Box>(graphics));
            }

            while (!window.is_closing())
            {

                graphics.clear();
                for (auto &box : boxes)
                {
                    box->update(0.016f);
                    box->draw(graphics);
                }
                graphics.end_frame();

                // graphics2.clear();
                // graphics2.end_frame();
                win::update();

                std::this_thread::sleep_for(16ms);
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