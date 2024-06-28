#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace gage::utils
{
    class Camera
    {   
    public:
        Camera();

        glm::mat4x4 get_view() const;
    private:
        glm::vec3 position{};
        glm::vec3 rotation{};  
    };
}