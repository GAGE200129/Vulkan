#pragma once

#include "Vulkan/Camera.hpp"

#include "Map/Map.hpp"

struct DebugData
{
    bool enabled;
    bool lockCamera;

    glm::vec3   cameraForward;
    Camera      camera;
    float       cameraSpeed;

    Box*        selectedBox;
};

namespace Debug
{
    
    void update();
    void render();
    void renderImgui();

    void debugCameraUpdate();

    void mapEditorRenderImgui();
    void mapEditorPickBrush();
    
    extern DebugData gData;
}