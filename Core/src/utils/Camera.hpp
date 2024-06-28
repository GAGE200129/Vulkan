#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace gage::utils
{
    class Camera
    {   
    public:
        Camera() = default;

        glm::mat4x4 get_view() const;

        glm::vec3& get_position();
        glm::vec3& get_rotation();
    private:
        glm::vec3 position{};
        glm::vec3 rotation{};  
    };
}