#include <stdio.h>

#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/gfx/data/g_buffer/GBuffer.hpp>
#include <Core/src/gfx/data/AmbientLight.hpp>
#include <Core/src/gfx/data/DirectionalLight.hpp>
#include <Core/src/gfx/data/PointLight.hpp>
#include <Core/src/gfx/data/SSAO.hpp>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/DebugRenderer.hpp>
#include <Core/src/utils/Cvar.hpp>

#include <Core/src/phys/phys.hpp>
#include <Core/src/phys/Physics.hpp>

#include <Core/src/scene/scene.hpp>
#include <Core/src/scene/SceneGraph.hpp>
#include <Core/src/scene/components/Animator.hpp>
#include <Core/src/scene/components/Map.hpp>
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
        gfx::Graphics gfx(window.p_window, "VulkanEngine");
        win::ImguiWindow imgui_window(gfx);

        hid::Keyboard keyboard(window.p_window);
        keyboard.register_action(hid::KeyCodes::W, "FORWARD");
        keyboard.register_action(hid::KeyCodes::A, "LEFT");
        keyboard.register_action(hid::KeyCodes::S, "BACKWARD");
        keyboard.register_action(hid::KeyCodes::D, "RIGHT");
        keyboard.register_action(hid::KeyCodes::SPACE, "JUMP");
        keyboard.register_action(hid::KeyCodes::LEFT_SHIFT, "SPRINT");
        keyboard.register_action(hid::KeyCodes::LEFT_ALT, "WALK");
        keyboard.register_action(hid::KeyCodes::Q, "LEAN_LEFT");
        keyboard.register_action(hid::KeyCodes::E, "LEAN_RIGHT");
        hid::Mouse mouse(window.p_window);

        gfx::data::Camera camera{};
        camera.far = 500.0f;
        camera.field_of_view = 90.0f;

        std::vector<gfx::data::PointLight::Data> point_lights{};

        // for (uint32_t x = 0; x < 5; x++)
        // {
        //     for (uint32_t y = 0; y < 5; y++)
        //     {
        //         point_lights.push_back({
        //             .position = {x * 30, 2, y * 30},
        //             .intensity = 100.0f,
        //             .color = {(float)x / 10.0f, (float)y / 10.0f, 1.0},
        //             .constant{1.0},
        //             .linear{0.35},
        //             .exponent{0.44},
        //         });
        //     }
        // }

        scene::SceneGraph scene(gfx, phys, camera);

        const scene::data::Model &scene_model = scene.import_model("res/models/human_base.glb", scene::data::ModelImportMode::Binary);

        scene::Node *animated_node = scene.instanciate_model(scene_model, {0, 0, 0});
        scene.add_component(animated_node, std::make_unique<scene::components::Animator>(scene, *animated_node, scene_model));
        animated_node->position = {50, 10, 50};
        animated_node->name = "Player";
        scene.add_component(animated_node, std::make_unique<scene::components::CharacterController>(scene, *animated_node, phys));
        scene.add_component(animated_node, std::make_unique<FPSCharacterController>(scene, *animated_node, phys, camera));

        auto terrain = scene.create_node();
        terrain->name = "TErrain";
        scene.add_component(terrain, std::make_unique<scene::components::Terrain>(scene, *terrain, gfx, 16, 17, 64, 1.0, 0, 1, 0.3f));

        auto map = scene.create_node();
        map->name = "Test map";
        map->position = {50.0f, 0.0f, 50.0f};
        scene::components::Map* map_comp = (scene::components::Map*)scene.add_component(map, std::make_unique<scene::components::Map>(scene, *map));

        scene::components::AABBWall aabb_wall{};
        aabb_wall.a = {0.0f, 0.0f, 0.0f};
        aabb_wall.b = {10.0f, 1.0f, 10.0f};
        aabb_wall.top.texture = "res/textures/grass_tiled.jpg";
        aabb_wall.top.uv_scale = {10.0f, 10.0f};
        aabb_wall.left.texture = "res/textures/x.jpg";
        aabb_wall.left.uv_scale = {10.0f, 10.0f};

        scene::components::StaticModel static_model{};
        static_model.model_path = "res/models/toothless.glb";

        map_comp->add_aabb_wall(aabb_wall);
        map_comp->add_aabb_wall({{0.0f, 10.0f, 10.0f}, {10.0f, 10.0f, 1.0f}});
        map_comp->add_aabb_wall({{0.0f, 10.0f, -10.0f}, {10.0f, 10.0f, 1.0f}});
        map_comp->add_static_model(static_model);
        

        scene.init();

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
                gfx.final_ambient->update(tick_time_in_seconds);

                phys.update(tick_time_in_seconds);
                scene.physics.update(tick_time_in_seconds);
                scene.animation.update(tick_time_in_seconds);
                scene.generic.update(tick_time_in_seconds, keyboard, mouse);

                scene.build_node_transform();

                scene.generic.late_update(tick_time_in_seconds, keyboard, mouse);
                scene.animation.late_update(tick_time_in_seconds);
                lag -= tick_time_in_nanoseconds;
            }

            mouse.update();
            win::update();

            imgui_window.clear();
            imgui_window.draw(camera, window, scene);
            imgui_window.end_frame();

            auto cmd = gfx.clear(camera);

            const auto &g_buffer = *gfx.geometry_buffer;

            g_buffer.begin_shadowpass(cmd);
            scene.renderer.render_depth(cmd);
            scene.terrain_renderer.render_depth(cmd);
            scene.map_renderer.render_depth(cmd);
            g_buffer.end(cmd);

            g_buffer.begin_mainpass(cmd);
            scene.renderer.render(cmd);
            scene.terrain_renderer.render(cmd);
            scene.map_renderer.render(cmd);
            g_buffer.end(cmd);

            g_buffer.begin_ssaopass(cmd);
            gfx.ssao->process(cmd);
            g_buffer.end(cmd);

            const auto &final_ambient = gfx.final_ambient;
            const auto &directional_light = gfx.directional_light;
            g_buffer.begin_lightpass(cmd);
            final_ambient->process(cmd);
            directional_light->process(cmd);

            for (const auto &point_light : point_lights)
            {
                gfx.point_light->process(cmd, point_light);
            }
            //gfx.debug_renderer->process(cmd);
            g_buffer.end(cmd);

            gfx.end_frame(cmd);
        }

        gfx.wait();
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