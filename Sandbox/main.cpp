#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/draw/Model.hpp>
#include <Core/src/gfx/data/GBuffer.hpp>
#include <Core/src/gfx/data/PBRPipeline.hpp>
#include <Core/src/gfx/data/FinalAmbient.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/terrain/Terrain.hpp>
#include <Core/src/utils/Cvar.hpp>

#include <thread>
#include <iostream>

#include <glm/gtx/string_cast.hpp>

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

            std::optional<gfx::draw::Model> model, model3;
            model.emplace(graphics, "res/models/sponza.glb", gfx::draw::Model::Mode::Binary);
            model3.emplace(graphics, "res/models/box_textured.glb", gfx::draw::Model::Mode::Binary);

            gfx::data::Camera camera{};
            camera.far = 100.0f;
            

            while (!window.is_closing())
            {
                win::update();
                imgui_window.clear();
                imgui_window.draw(camera, window);
                imgui_window.end_frame();

                auto frustum = camera.create_frustum(window.get_graphics().get_scaled_draw_extent().width, window.get_graphics().get_scaled_draw_extent().height);
                auto cmd = graphics.clear(camera);

                const auto &g_buffer = graphics.get_g_buffer();
                g_buffer.begin(cmd);

                const auto &pbr_pipeline = graphics.get_pbr_pipeline();
                const auto &final_ambient = graphics.get_final_ambient();
                pbr_pipeline.bind(cmd);
                model.value().draw(cmd, pbr_pipeline.get_layout(), frustum);
                model3.value().draw(cmd, pbr_pipeline.get_layout(), frustum);

                g_buffer.end(cmd);

                g_buffer.begin_finalpass(cmd);
                final_ambient.process(cmd);

                g_buffer.end(cmd);

                graphics.end_frame(cmd);
            }

            graphics.wait();
            model.reset();
            // model2.reset();
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