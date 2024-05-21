#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    inline glm::mat4 getProjection(const vk::Extent2D &extent) const
    {
        glm::mat4 proj = glm::perspectiveRH_ZO(glm::radians(fov), (float)extent.width / (float)extent.height,
                                               nearPlane, farPlane);
        return proj;
    }

    inline glm::mat4 getView() const
    {
        glm::mat4 result;

        result = glm::rotate(glm::mat4(1.0f), glm::radians(-pitch), {1, 0, 0});
        result = glm::rotate(result, glm::radians(-yaw), {0, 1, 0});
        result = glm::translate(result, -position);

        return result;
    }

public:
    glm::vec3 position;
    float pitch, yaw;
    float nearPlane, farPlane, fov;
};
