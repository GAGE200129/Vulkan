#include "Window.hpp"

#include "Exception.hpp"

#include <Core/src/log/Log.hpp>

#define GLFW_NO_API
#include <GLFW/glfw3.h>


namespace gage::win
{
 
    Window::Window(int width, int height, std::string title)
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, 0);
        p_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if(!p_window)
        {
            logger.error();
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

    void Window::resize(WindowMode mode, int width, int height, float scale)
    {
        switch(mode)
        {
        case WindowMode::Windowed:
        {
            glfwSetWindowMonitor(p_window, nullptr, 0, 0, width, height, 0);
            glfwSetWindowSize(p_window, width, height);
            p_graphics->set_resize(width, height);
            p_graphics->set_resolution_scale(scale);
            break;
        }
        case WindowMode::FullscreenBorderless:
        {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(p_window,  glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSetWindowSize(p_window, mode->width, mode->height);
            p_graphics->set_resize(mode->width, mode->height);
            p_graphics->set_resolution_scale(scale);
            break;
        }
        case WindowMode::FullscreenExclusive:
        {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(p_window,  glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSetWindowSize(p_window, width, height);
            p_graphics->set_resize(width, height);
            p_graphics->set_resolution_scale(scale);
            break;
        }

        default: break;
        }
    }

    gfx::Graphics& Window::get_graphics()
    {
        return *p_graphics.get();
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