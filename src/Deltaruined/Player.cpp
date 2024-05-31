#include "pch.hpp"
#include "Player.hpp"

#include "Input.hpp"
#include "EngineConstants.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

Player::Player() : mCharacter("res/models/box_textured.glb")
{
    mFocus = false;
    mCamera.position = {0, 0, 0};
    mCamera.nearPlane = 0.1f;
    mCamera.farPlane = 1000.0f;
    mCamera.pitch = 0;
    mCamera.yaw = 0;
    mCamera.fov = 60.0f;
}

void Player::update()
{
    if (Input::isKeyDownOnce(GLFW_KEY_ESCAPE))
        mFocus = !mFocus;

    Input::setCursorMode(mFocus);

    // Update cameara
    glm::vec3 forward;
    if (mFocus)
    {
        mCamera.pitch -= Input::getDy();
        mCamera.yaw -= Input::getDx();
        mCamera.pitch = glm::clamp(mCamera.pitch, -70.0f, 70.0f);
    }
    forward = (glm::mat3x3)glm::rotate(glm::mat4x4(1.0f), glm::radians(mCamera.pitch), {1.0f, 0.0f, 0.0f}) * glm::vec3(0, 0, -1);
    forward = (glm::mat3x3)glm::rotate(glm::mat4x4(1.0f), glm::radians(mCamera.yaw), {0.0f, 1.0f, 0.0f}) * forward;

    static glm::vec3 targetPosition = {0, 0, 0};
    targetPosition = glm::mix(targetPosition, mCharacter.getPosition(), EngineConstants::TICK_TIME * 10.0f);
    mCamera.position = targetPosition - forward * 5.0f;

    forward.y = 0;
    forward = glm::normalize(forward);
    glm::vec3 right = glm::cross(forward, {0, 1, 0});

    // Update character
    glm::vec3 moveDirection = {0, 0, 0};

    if (mFocus)
    {
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
    }

    if (glm::length2(moveDirection) != 0.0f)
    {
        moveDirection = glm::normalize(moveDirection);
    }
    mCharacter.setMoveDirection(moveDirection * 3.0f);
    mCharacter.update();
}

void Player::render() const
{
    mCharacter.render();
}
