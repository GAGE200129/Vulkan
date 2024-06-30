#include <stdio.h>

#include <Core/src/log/Log.hpp>
#include <Core/src/win/Window.hpp>
#include <Core/src/win/ImguiWindow.hpp>
#include <Core/src/gfx/Exception.hpp>
#include <Core/src/gfx/draw/Box.hpp>
#include <Core/src/gfx/bind/IBindable.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/ThirdParty/tiny_gltf.h>

#include <thread>

#include <Core/ThirdParty/imgui/imgui.h>

using namespace gage;
using namespace std::chrono_literals;

int main()
{
    using namespace tinygltf;

    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, "res/models/box_textured.glb");

    if (!warn.empty())
    {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty())
    {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret)
    {
        printf("Failed to parse glTF\n");
        return -1;
    }
    try
    {
        log::init();
        win::init();
        {
            win::Window window(800, 600, "Hello world");
            auto &graphics = window.get_graphics();
            win::ImguiWindow imgui_window{graphics};

            std::vector<std::unique_ptr<gfx::draw::Box>> boxes;

            for (int i = 0; i < 100; i++)
            {
                boxes.push_back(std::make_unique<gfx::draw::Box>(graphics));
            }

            gfx::data::Camera camera{};
            while (!window.is_closing())
            {

                static constexpr int64_t NS_PER_FRAME = (1.0 / 30.0) * 1000000000;
                auto start = std::chrono::high_resolution_clock::now();
                graphics.clear(camera);
                for (auto &box : boxes)
                {
                    box->update(0.016f);
                    box->draw(graphics);
                }
                graphics.end_frame();

                imgui_window.clear();
                imgui_window.draw(camera, window);
                imgui_window.end_frame();
                win::update();
                auto finish = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(start - finish).count();

                int64_t delay = duration + NS_PER_FRAME;
                if (delay > 0)
                    std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
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