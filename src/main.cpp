#include "pch.hpp"

#include "ECS/Components.hpp"

#include "ECS/GameObject.hpp"
#include "ECS/StaticModelComponent.hpp"
#include "Physics/BulletEngine.hpp"

#include "Asset/StaticModelLoader.hpp"

#include "EngineConstants.hpp"
#include "Input.hpp"
#include "Vulkan/Camera.hpp"

int main()
{
   
    BulletEngine::init();
    if(!VulkanEngine::init())
        return -1;
    Input::init(VulkanEngine::gData.window);

    Camera camera;
    camera.position = {0, 0, 0};
    camera.nearPlane = 0.1f;
    camera.farPlane = 100.0f;
    camera.pitch = 0;
    camera.yaw = 0;

    double lastTime = glfwGetTime();
    double lag = 0;
    while (!glfwWindowShouldClose(VulkanEngine::gData.window))
    {
        double current = glfwGetTime();
        double elapsed = current - lastTime;
        lastTime = current;

        lag += elapsed;
        while (lag >= EngineConstants::TICK_TIME)
        {
            glfwPollEvents();
            BulletEngine::update();
            Input::update();
            lag -= EngineConstants::TICK_TIME;
        }

        VulkanEngine::render(camera);
    }

    VulkanEngine::joint();
    StaticModelLoader::clearCache();
    VulkanEngine::cleanup();
    BulletEngine::cleanup();
   

    return 0;
}