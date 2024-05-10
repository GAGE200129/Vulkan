#pragma once

struct GLFWwindow;
class DebugCamera
{
public:
    DebugCamera(GLFWwindow *window) : mWindow(window) {}

    void update(float delta);

    glm::mat4 getPerspective();
    glm::mat4 getView();

private:
    GLFWwindow *mWindow;
    glm::vec3 mPosition = {0, 0, 0};
    float mPitch = 0.0f;
    float mYaw = 0.0f;
};