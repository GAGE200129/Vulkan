#pragma once

#include <string>
#include <memory>
#include <Core/src/gfx/Graphics.hpp>

namespace gage::gfx
{
    class Graphics;
}

struct GLFWwindow;
namespace gage::win
{
    
    class Window
    {
    public:
        Window(int width, int height, std::string title);
        ~Window();

        bool is_closing() const;

        gfx::Graphics& get_graphics();
    private:
        GLFWwindow* p_window;
        std::unique_ptr<gfx::Graphics> p_graphics;
    };

    void init();
    void update();
    void shutdown();
};