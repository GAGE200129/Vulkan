#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace gage::utils
{
    glm::mat4x4 Camera::get_view() const 
    {
        glm::mat4x4 view = glm::mat4x4(1.0f);
        view = glm::rotate(view, glm::radians(-rotation.x), {1.0f, 0.0f, 0.0f});
        view = glm::rotate(view, glm::radians(-rotation.y), {0.0f, 1.0f, 0.0f});
        view = glm::translate(view, -position);

        return view;
    }

    glm::vec3& Camera::get_position()
    {
        return position;
    }

    glm::vec3& Camera::get_rotation()
    {
        return rotation;
    }
}