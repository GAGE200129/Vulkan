#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>


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

        void resize(WindowMode mode, int width, int height);
    public:
        GLFWwindow* p_window{};
    };

    void init();
    void update();
    void shutdown();
};