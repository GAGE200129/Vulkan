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

    Frustum Camera::create_frustum(uint32_t width, uint32_t height) const
    {
        Frustum frustum;
        frustum.enabled = true;

        glm::vec3 global_front{0, 0, -1};
        glm::vec3 global_right{1, 0, 0};
        glm::vec3 global_up{0, 1, 0};
        glm::mat4x4 rotation_mat = glm::mat4x4(1.0f);
        rotation_mat = glm::rotate(rotation_mat, glm::radians(rotation.x), {1.0f, 0.0f, 0.0f});
        rotation_mat = glm::rotate(rotation_mat, glm::radians(rotation.y), {0.0f, 1.0f, 0.0f});

        global_up = glm::vec3(rotation_mat * glm::vec4(global_up, 0.0));
        global_front = glm::vec3(rotation_mat * glm::vec4(global_front, 0.0));
        global_right = glm::normalize(glm::cross(global_front, global_up));

        const float half_v_side = far * glm::tan(glm::radians(field_of_view * 0.5f));
        const float half_h_side = half_v_side * ((float)width / (float)height);
        const glm::vec3 front_mul_far = far * global_front;

        frustum.near = { global_front, position + near * global_front };
        frustum.far = { -global_front, position + front_mul_far };
        frustum.right = { glm::normalize(glm::cross(front_mul_far - global_right * half_h_side, global_up)), position };
        frustum.left = { glm::normalize(glm::cross(global_up, front_mul_far + global_right * half_h_side)), position };
        frustum.top = {  glm::normalize(glm::cross(global_right, front_mul_far - global_up * half_v_side)), position };
        frustum.bottom = { glm::normalize(glm::cross(front_mul_far + global_up * half_v_side, global_right)), position };

        return frustum;
    }
}