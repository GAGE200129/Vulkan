#pragma once

#include "Vulkan/Camera.hpp"

struct DebugData
{
    bool enabled;
    bool lockCamera;

    glm::vec3   cameraForward;
    Camera      camera;
    float       cameraSpeed;
};

namespace Debug
{
    
    void update();
    void render();
    void renderImgui();

    void debugCameraUpdate();

    void mapEditorRenderImgui();
    
    extern DebugData gData;
}