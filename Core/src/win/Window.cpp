#include <pch.hpp>
#include "Window.hpp"

#include "Exception.hpp"

#include <Core/src/utils/Cvar.hpp>
#include <Core/src/gfx/Graphics.hpp>

namespace gage::win
{
    
    Window::Window(int width, int height, std::string title)
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, 0);
        p_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if(!p_window)
        {
            throw WindowException{ "Failed to create window !" };
        }

        p_graphics = std::make_unique<gfx::Graphics>(p_window, title);
    }
    Window::~Window()
    {
        glfwDestroyWindow(p_window);
        p_graphics.reset();
    }

    bool Window::is_closing() const 
    {
        return glfwWindowShouldClose(p_window);
    }

    void Window::resize(WindowMode mode, int width, int height)
    {
        switch(mode)
        {
        case WindowMode::Windowed:
        {
            glfwSetWindowMonitor(p_window, nullptr, 0, 0, width, height, 0);
            glfwSetWindowSize(p_window, width, height);
            p_graphics->resize(width, height);
            break;
        }
        case WindowMode::FullscreenBorderless:
        {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(p_window,  glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSetWindowSize(p_window, mode->width, mode->height);
            p_graphics->resize(mode->width, mode->height);
            break;
        }
        case WindowMode::FullscreenExclusive:
        {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(p_window,  glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSetWindowSize(p_window, width, height);
            p_graphics->resize(width, height);
            break;
        }

        default: break;
        }
    }

    gfx::Graphics& Window::get_graphics()
    {
        return *p_graphics;
    }

    void init()
    {
        glfwInit();
        
    }

    void update()
    {
        glfwPollEvents();
    }
    void shutdown()
    {
        glfwTerminate();
    }
}