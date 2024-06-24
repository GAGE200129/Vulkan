#pragma once

#include <string>

struct GLFWwindow;
namespace gage::win
{
    class Window
    {
    public:
        Window(int width, int height, std::string title);
        ~Window();

        bool is_closing() const;
    private:
        GLFWwindow* p_window;
    };

    void init();
    void update();
    void shutdown();
};