#include <stdio.h>

#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/draw/Model.hpp>
#include <Core/src/gfx/data/g_buffer/GBuffer.hpp>
#include <Core/src/gfx/data/PBRPipeline.hpp>
#include <Core/src/gfx/data/ShadowPipeline.hpp>
#include <Core/src/gfx/data/AmbientLight.hpp>
#include <Core/src/gfx/data/DirectionalLight.hpp>
#include <Core/src/gfx/data/PointLight.hpp>
#include <Core/src/gfx/data/SSAO.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/terrain/Terrain.hpp>
#include <Core/src/utils/Cvar.hpp>

#include <Core/src/phys/phys.hpp>
#include <Core/src/phys/Physics.hpp>

#include <Core/src/scene/SceneGraph.hpp>

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
        gfx::init();
        win::init();
        phys::init();
        {
            phys::Physics phys;
            scene::SceneGraph scene;

            win::Window window(800, 600, "Hello world");
            auto &gfx = window.get_graphics();
            win::ImguiWindow imgui_window{gfx};

            std::optional<gfx::draw::Model> model, model3;
            model.emplace(gfx, "res/models/sponza.glb", gfx::draw::Model::Mode::Binary);
            model3.emplace(gfx, "res/models/box_textured.glb", gfx::draw::Model::Mode::Binary);

            gfx::data::Camera camera{};
            camera.far = 100.0f;

            std::vector<gfx::data::PointLight::Data> point_lights{};

            
            // gfx::data::PointLight::Data point_light{};
            //  point_light.position.x = 0;
            //  point_light.position.z = 0;
            //  point_light.position.y = 5;
            //  point_light.color = { (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX};

            //  point_lights.push_back(point_light);

            //for(int x = -10; x <= 10; x += 4)
            //{
            //    for(int z = -4; z <= 4; z += 1)
            //    {
            //        gfx::data::PointLight::Data point_light{};
            //        point_light.position.x = x;
            //        point_light.position.z = z;
            //        point_light.position.y = 5;
            //        point_light.color = { (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX};
            //        point_lights.push_back(point_light);
            //    }
            //}
            

            while (!window.is_closing())
            {
                win::update();
                imgui_window.clear();
                imgui_window.draw(camera, window, scene);
                imgui_window.end_frame();

                auto frustum = camera.create_frustum(gfx.get_scaled_draw_extent().width, gfx.get_scaled_draw_extent().height);
                auto cmd = gfx.clear(camera);

                const auto &g_buffer = gfx.get_g_buffer();
                const auto &pbr_pipeline = gfx.get_pbr_pipeline();
                const auto &shadow_pipeline = gfx.get_shadow_pipeline();


                
                g_buffer.begin_shadowpass(cmd);
                shadow_pipeline.bind(cmd);
                model.value().draw(cmd, shadow_pipeline.get_layout(), glm::mat4x4(1.0f), frustum);
                model3.value().draw(cmd, shadow_pipeline.get_layout(), glm::mat4x4(1.0f), frustum);
                g_buffer.end(cmd);
                
                g_buffer.begin_mainpass(cmd);
                pbr_pipeline.bind(cmd);
                model.value().draw(cmd, pbr_pipeline.get_layout(), glm::mat4x4(1.0f), frustum);
                model3.value().draw(cmd, pbr_pipeline.get_layout(), glm::mat4x4(1.0f), frustum);

                g_buffer.end(cmd);
                
                g_buffer.begin_ssaopass(cmd);
                gfx.get_ssao().process(cmd);
                g_buffer.end(cmd);


                const auto &final_ambient = gfx.get_final_ambient();
                const auto &directional_light = gfx.get_directional_light();
                const auto &point_light = gfx.get_point_light();
                g_buffer.begin_lightpass(cmd);
                final_ambient.process(cmd);
                directional_light.process(cmd);
                for(auto& p_light : point_lights)
                {
                    point_light.process(cmd, p_light);
                }

                g_buffer.end(cmd);

                gfx.end_frame(cmd);
            }

            gfx.wait();
            model.reset();
            // model2.reset();
            model3.reset();

        }
        gfx::shutdown();
        win::shutdown();
        phys::shutdown();
    }
    catch (gfx::GraphicsException &e)
    {
        std::cerr << "Graphics exception caught: " + std::string(e.what()) << "\n";
    }
    catch (utils::FileLoaderException &e)
    {
        std::cerr << "File loader exception caught: " + std::string(e.what()) << "\n";
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught !" << "\n";
    }

    return 0;
}