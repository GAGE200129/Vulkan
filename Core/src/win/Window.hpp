#pragma once

#include <string>
#include <vulkan/vulkan_core.h>

struct GLFWwindow;
namespace gage::win
{
    class Window
    {
    public:
        Window(int width, int height, std::string title);
        ~Window();
    private:
        VkInstance instance;
        VkSurfaceKHR surface;
        GLFWwindow* p_window;
    };

    void init();
    void shutdown();
};