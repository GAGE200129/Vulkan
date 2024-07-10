#include <pch.hpp>
#include "Camera.hpp"


namespace gage::gfx::data
{
    glm::mat4x4 Camera::get_view() const 
    {
        glm::mat4x4 view = glm::mat4x4(1.0f);
        view = glm::rotate(view, glm::radians(-rotation.x), {1.0f, 0.0f, 0.0f});
        view = glm::rotate(view, glm::radians(-rotation.y), {0.0f, 1.0f, 0.0f});
        view = glm::translate(view, -position);

        return view;
    }

    const glm::vec3& Camera::get_position() const 
    {
        return position;
    }

    const  glm::vec3& Camera::get_rotation() const 
    {
        return rotation;
    }

    glm::vec3& Camera::get_position()
    {
        return position;
    }

    glm::vec3& Camera::get_rotation()
    {
        return rotation;
    }

    float Camera::get_field_of_view() const
    {
        return field_of_view;
    }
    float Camera::get_near() const
    {
        return near;
    }
    float Camera::get_far() const
    {
        return far;
    }
}