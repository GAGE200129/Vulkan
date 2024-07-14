#pragma once

#include <glm/vec3.hpp>

namespace gage::gfx::data
{
    struct Plane
    {
        glm::vec3 normal{};
        glm::vec3 point{};
    };

    struct Frustum
    {
        bool enabled{false};
        Plane top{};
        Plane bottom{};

        Plane right{};
        Plane left{};

        Plane far{};
        Plane near{};
    };
}