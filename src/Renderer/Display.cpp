#include "pch.hpp"
#include "Renderer.hpp"

#include "EngineConstants.hpp"

bool Renderer::displayInit()
{
    spdlog::info("Creating a window !");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    gData.window = glfwCreateWindow(EngineConstants::DISPLAY_WIDTH, EngineConstants::DISPLAY_HEIGHT,
         EngineConstants::DISPLAY_TITLE, nullptr, nullptr);
    if(!gData.window)
    {
        spdlog::critical("Failed to create vulkan window !");
        return false;
    }

    return true;
}
