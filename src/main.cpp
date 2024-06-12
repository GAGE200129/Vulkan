#include "pch.hpp"

#include "Physics/BulletEngine.hpp"
#include "Input.hpp"
#include "Vulkan/VulkanEngine.hpp"
#include "Asset/StaticModelLoader.hpp"
#include "Debug/Debug.hpp"

#include "Deltaruined/Player.hpp"
#include "Map/Map.hpp"
#include "Renderer/Renderer.hpp"

#include <imgui.h>

int main()
{
    using namespace std::chrono_literals;


    if(!Renderer::init())
        return -2;

    BulletEngine::init();
    if(!VulkanEngine::initWindow())
        return -1;
    Input::init(VulkanEngine::gData.window);
    if (!VulkanEngine::init())
        return -1;
    VulkanEngine::raymarchInit();
    Map::init();

   


    Box b = {};
    b.center = {0, 0, 0};
    b.halfSize = {1.5f, 0.5f, 1.5f};
    std::strncpy(b.faces[0].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
    b.faces[0].scaleX = 0.01f;
    b.faces[0].scaleY = 0.01f;

    std::strncpy(b.faces[1].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
    b.faces[1].scaleX = 0.01f;
    b.faces[1].scaleY = 0.01f;

    std::strncpy(b.faces[2].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
    b.faces[2].scaleX = 0.01f;
    b.faces[2].scaleY = 0.01f;

    std::strncpy(b.faces[3].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
    b.faces[3].scaleX = 0.01f;
    b.faces[3].scaleY = 0.01f;

    std::strncpy(b.faces[4].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
    b.faces[4].scaleX = 0.01f;
    b.faces[4].scaleY = 0.01f;

    std::strncpy(b.faces[5].texturePath, "res/textures/x.jpg", EngineConstants::PATH_LENGTH);
    b.faces[5].scaleX = 0.01f;
    b.faces[5].scaleY = 0.01f;


    Map::boxAdd(b);

    Player player;

    Character character("res/models/death.glb");
    btTransform &bt = character.getBody()->getWorldTransform();
    bt.setOrigin(btVector3(3, 0, 0));

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
            Debug::update();

            if (!Debug::gData.enabled)
            {
                BulletEngine::update();
                player.update();
                character.update();
            }
            VulkanEngine::raymarchUpdate();
            Input::update();
            lag -= EngineConstants::TICK_TIME;
        }

        auto frameStart = std::chrono::high_resolution_clock::now();

        Renderer::render();

        VulkanEngine::widgetBeginFrame();
        Debug::renderImgui();

        const Camera& currentCamera = Debug::gData.enabled ? Debug::gData.camera : player.getCamera();
        VulkanEngine::beginFrame(currentCamera);
        //VulkanEngine::raymarchRender(currentCamera);
        player.render();
        character.render();
        Map::render();
        
        
        Debug::render();
        VulkanEngine::endFrame();

        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
        int64_t overhead = EngineConstants::FRAME_TIME * 1000000.0 - dt.count();
        if (overhead > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(overhead)); // Lock fps 
    }

    VulkanEngine::joint();
    VulkanEngine::raymarchCleanup();
    Map::cleanup();
    StaticModelLoader::clearCache();
    VulkanEngine::cleanup();
    Renderer::cleanup();
    BulletEngine::cleanup();

    return 0;
}