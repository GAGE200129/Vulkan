#include "pch.hpp"
#include "Debug.hpp"

#include "Input.hpp"
#include "EngineConstants.hpp"

#include <glm/gtx/norm.hpp>


void Debug::debugCameraUpdate()
{
    if (gData.lockCamera)
    {
        Input::setCursorMode(true);
        gData.camera.pitch -= Input::getDy();
        gData.camera.yaw -= Input::getDx();
        gData.camera.pitch = glm::clamp(gData.camera.pitch, -89.0f, 89.0f);

        glm::vec3 forward;
        forward = (glm::mat3x3)glm::rotate(glm::mat4x4(1.0f), glm::radians(gData.camera.pitch), {1.0f, 0.0f, 0.0f}) * glm::vec3(0, 0, -1);
        forward = (glm::mat3x3)glm::rotate(glm::mat4x4(1.0f), glm::radians(gData.camera.yaw), {0.0f, 1.0f, 0.0f}) * forward;
        gData.cameraForward = forward;
        forward.y = 0;
        forward = glm::normalize(forward);
        glm::vec3 right = glm::cross(forward, {0, 1, 0});

        // Update character
        glm::vec3 moveDirection = {0, 0, 0};
        if (Input::isKeyDown(GLFW_KEY_W))
        {
            moveDirection += forward;
        }

        if (Input::isKeyDown(GLFW_KEY_D))
        {
            moveDirection += right;
        }
        if (Input::isKeyDown(GLFW_KEY_S))
        {
            moveDirection -= forward;
        }

        if (Input::isKeyDown(GLFW_KEY_A))
        {
            moveDirection -= right;
        }
        if (Input::isKeyDown(GLFW_KEY_SPACE))
        {
            moveDirection.y += 1.0f;
        }

        if (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            moveDirection.y -= 1.0f;
        }

        if (glm::length2(moveDirection) != 0.0f)
        {
            moveDirection = glm::normalize(moveDirection);
        }

        gData.camera.position += moveDirection * gData.cameraSpeed * (float)EngineConstants::TICK_TIME;
    }
    else
    {
        Input::setCursorMode(false);
    }
}