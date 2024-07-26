
#include <pch.hpp>
#include "Mouse.hpp"

namespace gage::hid
{
    Mouse::Mouse(GLFWwindow* p_window) : p_window(p_window){}
    Mouse::~Mouse() {}

    void Mouse::update()
    {
        prev_mouse_pos =  get_position();
    }

    glm::ivec2 Mouse::get_position() const
    {
        double x_pos, y_pos;
        glfwGetCursorPos(p_window, &x_pos, &y_pos);
        return {(uint32_t)x_pos, (uint32_t)y_pos};
    }

    glm::ivec2 Mouse::get_delta() const
    {
        return get_position() - prev_mouse_pos;
    }
}