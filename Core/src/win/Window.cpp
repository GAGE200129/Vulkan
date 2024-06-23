#include "Window.hpp"

#include "Exception.hpp"


#include <Core/src/log/Log.hpp>

#define GLFW_NO_API
#include <GLFW/glfw3.h>


namespace gage::win
{

    Window::Window(int width, int height, std::string title)
    {
        p_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if(!p_window)
        {
            logger.error("Failed to create window !");
            throw WindowException{ "Failed to create window !" };
        }
        

    }
    Window::~Window()
    {
        glfwDestroyWindow(p_window);
    }

    void init()
    {
        glfwInit();
    }
    void shutdown()
    {
        glfwTerminate();
    }
}