#include <stdio.h>

#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/data/g_buffer/GBuffer.hpp>
#include <Core/src/gfx/data/AmbientLight.hpp>
#include <Core/src/gfx/data/DirectionalLight.hpp>
#include <Core/src/gfx/data/PointLight.hpp>
#include <Core/src/gfx/data/SSAO.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/utils/Cvar.hpp>

#include <Core/src/phys/phys.hpp>
#include <Core/src/phys/Physics.hpp>

#include <Core/src/scene/scene.hpp>
#include <Core/src/scene/SceneGraph.hpp>
#include <Core/src/scene/components/Animator.hpp>
#include "scripts/FPSCharacterController.hpp"

#include <Core/src/hid/hid.hpp>
#include <Core/src/hid/Keyboard.hpp>
#include <Core/src/hid/Mouse.hpp>

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
    hid::init();
    try
    {

        phys::Physics phys;

        win::Window window(800, 600, "Hello world");
        hid::Keyboard keyboard(window.get_handle());
        keyboard.register_action(hid::KeyCodes::W, "FORWARD");
        keyboard.register_action(hid::KeyCodes::A, "LEFT");
        keyboard.register_action(hid::KeyCodes::S, "BACKWARD");
        keyboard.register_action(hid::KeyCodes::D, "RIGHT");
        keyboard.register_action(hid::KeyCodes::SPACE, "JUMP");
        keyboard.register_action(hid::KeyCodes::LEFT_SHIFT, "SPRINT");
        keyboard.register_action(hid::KeyCodes::LEFT_ALT, "WALK");
        keyboard.register_action(hid::KeyCodes::Q, "LEAN_LEFT");
        keyboard.register_action(hid::KeyCodes::E, "LEAN_RIGHT");
        hid::Mouse mouse(window.get_handle());

        auto &gfx = window.get_graphics();
        win::ImguiWindow imgui_window{gfx};

        gfx::data::Camera camera{};
        camera.far = 500.0f;
        camera.field_of_view = 90.0f;

        std::vector<gfx::data::PointLight::Data> point_lights{};

        std::optional<scene::SceneGraph> scene;
        scene.emplace(gfx, phys, camera);


        const scene::data::Model &scene_model = scene->import_model("res/models/human_base.glb", scene::SceneGraph::ImportMode::Binary);

        scene::Node *animated_node = scene->instanciate_model(scene_model, {0, 0, 0});
        scene->add_component(animated_node, std::make_unique<scene::components::Animator>(*scene, *animated_node, scene_model));
        animated_node->set_position({50, 150, 50});
        animated_node->set_name("Player");
        scene->add_component(animated_node, std::make_unique<scene::components::CharacterController>(*scene, *animated_node, phys));
        scene->add_component(animated_node, std::make_unique<FPSCharacterController>(*scene, *animated_node, phys, camera));

        auto terrain = scene->create_node();
        terrain->set_name("TErrain");
        scene->add_component(terrain, std::make_unique<scene::components::Terrain>(*scene, *terrain, gfx, 64, 17, 64, 1.0, 0, 100, 0.3f));

        scene->init();

        auto previous = std::chrono::high_resolution_clock::now();
        uint64_t lag = 0;

        double tick_time_in_seconds = 1.0 / 128.0;
        uint64_t tick_time_in_nanoseconds = tick_time_in_seconds * 1E9;

        while (!window.is_closing())
        {
            auto current = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::nanoseconds(current - previous);
            previous = current;
            lag += elapsed.count();
 
            while (lag >= tick_time_in_nanoseconds)
            {
                gfx.get_final_ambient().update(tick_time_in_seconds);
                
                phys.update(tick_time_in_seconds);
                scene->get_physics().update(tick_time_in_seconds);
                scene->get_animation().update(tick_time_in_seconds);
                scene->get_generic().update(tick_time_in_seconds, keyboard, mouse);

                scene->build_node_transform();

                scene->get_generic().late_update(tick_time_in_seconds, keyboard, mouse);
                scene->get_animation().late_update(tick_time_in_seconds);
                lag -= tick_time_in_nanoseconds;
            }

            mouse.update();
            win::update();

            imgui_window.clear();
            imgui_window.draw(camera, window, *scene);
            imgui_window.end_frame();

            auto cmd = gfx.clear(camera);

            const auto &g_buffer = gfx.get_g_buffer();


            g_buffer.begin_shadowpass(cmd);
            scene->get_renderer().render_depth(cmd);
            scene->get_terrain_renderer().render_depth(cmd);
            g_buffer.end(cmd);

            g_buffer.begin_mainpass(cmd);
            scene->get_renderer().render(cmd);
            scene->get_terrain_renderer().render(cmd);
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
    hid::shutdown();
    scene::shutdown();
    gfx::shutdown();
    win::shutdown();
    phys::shutdown();

    return 0;
}