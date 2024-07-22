#include <stdio.h>

#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/data/g_buffer/GBuffer.hpp>
#include <Core/src/gfx/data/PBRPipeline.hpp>
#include <Core/src/gfx/data/ShadowPipeline.hpp>
#include <Core/src/gfx/data/AmbientLight.hpp>
#include <Core/src/gfx/data/DirectionalLight.hpp>
#include <Core/src/gfx/data/PointLight.hpp>
#include <Core/src/gfx/data/SSAO.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/terrain/Terrain.hpp>
#include <Core/src/gfx/data/terrain/TerrainPipeline.hpp>
#include <Core/src/utils/Cvar.hpp>

#include <Core/src/phys/phys.hpp>
#include <Core/src/phys/Physics.hpp>

#include <Core/src/scene/scene.hpp>
#include <Core/src/scene/SceneGraph.hpp>
#include <Core/src/scene/components/Animator.hpp>

#include <thread>
#include <iostream>

#include <glm/gtx/string_cast.hpp>

#include <Core/ThirdParty/imgui/imgui.h>

using namespace gage;
using namespace std::chrono_literals;

int main()
{
    gfx::init();
    win::init();
    phys::init();
    scene::init();
    try
    {

        phys::Physics phys;
        

        win::Window window(800, 600, "Hello world");
        auto &gfx = window.get_graphics();
        win::ImguiWindow imgui_window{gfx};

        gfx::data::Camera camera{};
        camera.far = 1000.0f;

        //std::optional<gfx::data::terrain::Terrain> new_terrain;
        //new_terrain.emplace(gfx, "res/terrains/test.ter");

        std::vector<gfx::data::PointLight::Data> point_lights{};

        std::optional<scene::SceneGraph> scene;
        scene.emplace(gfx);

        const scene::data::Model& scene_model = scene->import_model("res/models/toothless.glb", scene::SceneGraph::ImportMode::Binary);
        const scene::data::Model& sponza_model = scene->import_model("res/models/sponza.glb", scene::SceneGraph::ImportMode::Binary);


        scene::Node* animated_node = scene->instanciate_model(scene_model, {0, 0, 0});
        scene::Node* animated_node2 = scene->instanciate_model(scene_model, {2, 0, 0});
        scene->instanciate_model(sponza_model, {0, 0, 0});

        scene::components::Animator* animator = (scene::components::Animator*)animated_node->get_requested_component(typeid(scene::components::Animator).name());
        scene::components::Animator* animator2 = (scene::components::Animator*)animated_node2->get_requested_component(typeid(scene::components::Animator).name());
        


        scene->init();


        animator->set_current_animation("Armature|mixamo.com|Layer0");
        animator2->set_current_animation("Armature|mixamo.com|Layer0");
        while (!window.is_closing())
        {
            scene->update(0.016f);

            win::update();
            imgui_window.clear();
            imgui_window.draw(camera, window, *scene);
            imgui_window.end_frame();

            auto frustum = camera.create_frustum(gfx.get_scaled_draw_extent().width, gfx.get_scaled_draw_extent().height);
            auto cmd = gfx.clear(camera);

            const auto &g_buffer = gfx.get_g_buffer();
            const auto &pbr_pipeline = gfx.get_pbr_pipeline();
            const auto &terrain_pipeline = gfx.get_terrain_pipeline();

            g_buffer.begin_shadowpass(cmd);
            pbr_pipeline.bind_depth(cmd);
            scene->render_depth(cmd, pbr_pipeline.get_depth_layout());
            //new_terrain->render(gfx, cmd);
            g_buffer.end(cmd);

            g_buffer.begin_mainpass(cmd);
            pbr_pipeline.bind(cmd);
            scene->render_geometry(cmd, pbr_pipeline.get_layout());
            
            terrain_pipeline.bind(cmd);
            //new_terrain->render(gfx, cmd);
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
            for (auto &p_light : point_lights)
            {
                point_light.process(cmd, p_light);
            }

            g_buffer.end(cmd);

            gfx.end_frame(cmd);
        }

        gfx.wait();
        scene.reset();
    }
    catch (gfx::GraphicsException &e)
    {
        std::cerr << "Graphics exception caught: " + std::string(e.what()) << "\n";
    }
    catch (scene::SceneException &e)
    {
        std::cerr << "Scene exception caught: " + std::string(e.what()) << "\n";
    }
    catch (utils::FileLoaderException &e)
    {
        std::cerr << "File loader exception caught: " + std::string(e.what()) << "\n";
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught !" << "\n";
    }

    scene::shutdown();
    gfx::shutdown();
    win::shutdown();
    phys::shutdown();

    return 0;
}