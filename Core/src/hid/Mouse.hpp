#pragma once

#include <glm/vec2.hpp>

struct GLFWwindow;
namespace gage::hid
{
    class Mouse
    {
    public:
        Mouse(GLFWwindow* p_window);
        ~Mouse();

        void update();

        glm::ivec2 get_position() const;
        glm::ivec2 get_delta() const;
    private:
        GLFWwindow* p_window;
        glm::ivec2 prev_mouse_pos{};
    };
};