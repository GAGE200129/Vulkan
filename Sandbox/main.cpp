#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/draw/Model.hpp>

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


            std::optional<gfx::draw::Model> model, model2, model3;
            model.emplace(graphics, "res/models/sponza.glb", gfx::draw::Model::Mode::Binary);
            //model2.emplace(graphics, "res/models/box_textured.glb", gfx::draw::Model::Mode::Binary);
            model3.emplace(graphics, "res/models/DamagedHelmet.gltf", gfx::draw::Model::Mode::ASCII);


            gfx::data::Camera camera{};
            while (!window.is_closing())
            {

                static constexpr int64_t NS_PER_FRAME = (1.0 / 60.0) * 1000000000;
                auto start = std::chrono::high_resolution_clock::now();
                auto cmd = graphics.clear(camera);
                graphics.get_default_pipeline().begin(cmd);
                model.value().draw(cmd);
                //model2.value().draw(cmd);
                model3.value().draw(cmd);
                graphics.get_default_pipeline().end(cmd);

                graphics.end_frame();

                imgui_window.clear();
                imgui_window.draw(camera, window);
                imgui_window.end_frame();
                win::update();
                auto finish = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(start - finish).count();

                int64_t delay = NS_PER_FRAME - duration;
                if (delay > 0)
                    std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
            }

            graphics.wait();
            model.reset();
            //model2.reset();
            model3.reset();

        }
        win::shutdown();
    }
    catch (gfx::GraphicsException &e)
    {
        logger.error("Graphics exception caught: " + std::string(e.what()));
    }
    catch (utils::FileLoaderException &e)
    {
        logger.error("Utils exception caught: " + std::string(e.what()));
    }
    catch (...)
    {
        logger.info("Unknown exception caught.");
    }

    return 0;
}