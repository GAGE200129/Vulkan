#include "pch.hpp"
#include "Debug.hpp"

#include "Input.hpp"

#include <imgui.h>

#include "EngineConstants.hpp"

DebugData Debug::gData =
    {
        .enabled = false,
        .lockCamera = false,
        .cameraForward = {0, 0, -1},
        .camera =
            {
                .position = {0, 0, 0},
                .pitch = 0,
                .yaw = 0,
                .nearPlane = 0.1f,
                .farPlane = 1000.0f,
                .fov = 90.0f
            },
        .cameraSpeed = 5.0f,
        .selectedBox = nullptr
    };

void Debug::update()
{
    if (Input::isKeyDownOnce(GLFW_KEY_F5))
        gData.enabled = !gData.enabled;

    if (Input::isKeyDownOnce(GLFW_KEY_ESCAPE))
        gData.lockCamera = !gData.lockCamera;

    ImGuiIO &io = ImGui::GetIO();
    if (!gData.enabled || io.WantCaptureKeyboard || io.WantCaptureMouse)
        return;
    debugCameraUpdate();
    mapEditorPickBrush();
}

void Debug::renderImgui()
{
    if (!gData.enabled)
        return;
    ImGui::Begin("Debugger");
    ImGui::SliderFloat("Camera speed", &gData.cameraSpeed, 1.0f, 10.0f);
    ImGui::Text("%p", gData.selectedBox);
    ImGui::End();   

    ImGui::ShowDemoWindow();
    mapEditorRenderImgui();
}

void Debug::render()
{
    if (!gData.enabled)
        return;
}