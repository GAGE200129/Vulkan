#include "pch.hpp"

#include "Vulkan/VulkanTexture.hpp"

#include "ECS/Components.hpp"

#include "ECS/GameObject.hpp"
#include "ECS/ModelComponent.hpp"
#include "ECS/ScriptComponent.hpp"
#include "ECS/AnimatedModelComponent.hpp"
#include "ECS/AnimatorComponent.hpp"
#include "ECS/RigidBodyComponent.hpp"
#include "ECS/BoxColliderComponent.hpp"
#include "ECS/CharacterControllerComponent.hpp"
#include "Physics/BulletEngine.hpp"

#include "EngineConstants.hpp"
#include "Input.hpp"


extern void debugMain();
extern bool gDebugPaused;

static void windowResizeFn(GLFWwindow *window, int width, int height) noexcept
{
    VulkanEngine::onWindowResize(width, height);
}

GLFWwindow *gMainWindow;


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    gMainWindow = glfwCreateWindow(1600, 900, "Vulkan", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(gMainWindow, windowResizeFn);
    BulletEngine::init();
    if(!VulkanEngine::init(gMainWindow))
        return -1;
    Input::init(gMainWindow);

    std::vector<AnimatorComponent *> animators;

    for (int i = 5; i < 15; i += 2)
    {
        for (int j = 5; j < 15; j += 2)
        {
            GameObject *go = &GameObject::addGameObject("Testing");
            animators.push_back(go->addComponent<AnimatorComponent>());
            go->addComponent<AnimatedModelComponent>("res/models/box.glb");
            go->addComponent<BoxColliderComponent>(glm::vec3{0.0f, 3.0f, 0.0f}, glm::vec3{1.0f, 3.0f, 1.0f});
            go->addComponent<RigidBodyComponent>(1.0f);
            go->mPosition.x = i;
            go->mPosition.z = j;
        }
    }

    GameObject &go1 = GameObject::addGameObject("Testing1");
    go1.addComponent<ModelComponent>("res/models/box_textured.glb");
    go1.addComponent<CharacterControllerComponent>();
    go1.addComponent<ScriptComponent>("res/scripts/testing.lua");

    GameObject::globalInit();

    // Launch debugger
    std::unique_ptr<std::thread> debugThread;
    if (EngineConstants::DEBUG_ENABLED)
        debugThread = std::make_unique<std::thread>(debugMain);

    double lastTime = glfwGetTime();
    double lag = 0;
    while (!glfwWindowShouldClose(gMainWindow))
    {
        double current = glfwGetTime();
        double elapsed = current - lastTime;
        lastTime = current;

        if (gDebugPaused)
            continue;
        lag += elapsed;
        while (lag >= EngineConstants::TICK_TIME)
        {
            glfwPollEvents();
            BulletEngine::update();
            GameObject::globalUpdate();
            Input::update();
            lag -= EngineConstants::TICK_TIME;
        }

        VulkanEngine::render();
    }
    if (EngineConstants::DEBUG_ENABLED)
        debugThread->join();

    VulkanEngine::joint();

    GameObject::globalShutdown();
    ModelComponent::clearCache();
    AnimatedModelComponent::clearCache();
    VulkanEngine::cleanup();
    BulletEngine::cleanup();
    glfwDestroyWindow(gMainWindow);
    glfwTerminate();

    return 0;
}