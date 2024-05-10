#include "pch.hpp"
#include "DebugCamera.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include <imgui.h>

void DebugCamera::update(float delta)
{
    static glm::vec2 prevMousePos = {0, 0};
    static glm::vec2 deltaMouse = {0, 0};

    double x, y;
    glfwGetCursorPos(mWindow, &x, &y);
    deltaMouse.x = x - prevMousePos.x;
    deltaMouse.y = y - prevMousePos.y;
    prevMousePos.x = x;
    prevMousePos.y = y;

    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    bool leftMouseClicked = glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (leftMouseClicked)
    {
        mYaw -= deltaMouse.x;
        mPitch -= deltaMouse.y;
    }

    glm::vec3 forward = glm::rotate(glm::vec3(0.0f, 0.0f, -1.0f), glm::radians(mYaw), glm::vec3{0.0f, 1.0f, 0.0f});
    glm::vec3 right = glm::rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(mYaw), glm::vec3{0.0f, 1.0f, 0.0f});
    glm::vec3 direction = {0, 0, 0};
    if (glfwGetKey(mWindow, GLFW_KEY_W) && leftMouseClicked)
    {
        direction += forward;
    }

    if (glfwGetKey(mWindow, GLFW_KEY_S) && leftMouseClicked)
    {
        direction -= forward;
    }

    if (glfwGetKey(mWindow, GLFW_KEY_D) && leftMouseClicked)
    {
        direction += right;
    }

    if (glfwGetKey(mWindow, GLFW_KEY_A) && leftMouseClicked)
    {
        direction -= right;
    }

    if (glfwGetKey(mWindow, GLFW_KEY_SPACE) && leftMouseClicked)
    {
        direction.y += 1;
    }

    if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) && leftMouseClicked)
    {
        direction.y -= 1;
    }

    if (glm::length2(direction) != 0.0f)
    {
        direction = glm::normalize(direction);
        mPosition += direction * delta;
    }
}

glm::mat4 DebugCamera::getPerspective()
{
    int width, height;
    glfwGetFramebufferSize(mWindow, &width, &height);
    return glm::perspective(glm::radians(70.0f), (float)width / (float)height, 0.1f, 100.0f);
}
glm::mat4 DebugCamera::getView()
{
    glm::mat4 result;

    result = glm::rotate(glm::mat4(1.0f), glm::radians(-mPitch), {1, 0, 0});
    result = glm::rotate(result, glm::radians(-mYaw), {0, 1, 0});
    result = glm::translate(result, -mPosition);

    return result;
}
