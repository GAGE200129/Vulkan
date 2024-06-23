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
            logger.error();
            throw WindowException{ "Failed to create window !" };
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = title.c_str();
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
            
        if (VkResult result = vkCreateInstance(&createInfo, nullptr, &instance); result != VK_SUCCESS) 
        {
            logger.error().vk_result(result);
            throw VulkanException("failed to create instance!");
        }

    }
    Window::~Window()
    {
        vkDestroyInstance(instance, nullptr);
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