#pragma once

#include <string>

#include <Core/ThirdParty/VkBootstrap.h>

struct GLFWwindow;

namespace gage::gfx::data
{
    class Instance
    {
    public:
        Instance(const std::string& app_name, GLFWwindow* window);
        ~Instance();
    public:
        vkb::Instance vkb_instance{};
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};
        VkSurfaceKHR surface{};
    };
}