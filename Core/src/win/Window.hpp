#pragma once

#include <string>
#include <memory>
#include <optional>
#include <Core/src/gfx/Graphics.hpp>

namespace gage::gfx
{
    class Graphics;
}

struct GLFWwindow;
namespace gage::win
{
    enum class WindowMode
    {
        Windowed = 0,
        FullscreenBorderless,
        FullscreenExclusive
    };
    class Window
    {
    public:
        Window(int width, int height, std::string title);
        ~Window();

        bool is_closing() const;

        void resize(WindowMode mode, int width, int height, float scale);

        gfx::Graphics& get_graphics();
    private:
        GLFWwindow* p_window{};
        std::optional<gfx::Graphics> p_graphics{};
    };

    void init();
    void update();
    void shutdown();
};