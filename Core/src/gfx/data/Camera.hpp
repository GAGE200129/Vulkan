#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace gage::gfx::data
{
    class Camera
    {   
    public:
        Camera() = default;

        glm::mat4x4 get_view() const;

        const glm::vec3& get_position() const;
        const glm::vec3& get_rotation() const;

        glm::vec3& get_position();
        glm::vec3& get_rotation();

        float get_field_of_view() const;
        float get_near() const;
        float get_far() const;
    private:
        glm::vec3 position{};
        glm::vec3 rotation{};
        
        float field_of_view{90.0f};
        float near{0.1f};
        float far{1000.0f};

    };
}