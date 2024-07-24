#include <pch.hpp>
#include "Keyboard.hpp"

#include "hid.hpp"

namespace gage::hid
{
    Keyboard::Keyboard(GLFWwindow* p_window) :
        p_window(p_window)
    {

        
    }

    Keyboard::~Keyboard() {}

    void Keyboard::register_action(KeyCodes key_code, const std::string& name)
    {
        if(action_map.find(name) != action_map.end())
        {
            log().info("Overriding action: {}, with: {}", (uint32_t)action_map.at(name), (uint32_t)key_code);
        }
        action_map[name] = key_code;
    }

    bool Keyboard::is_key_down(uint32_t glfw_key_code) const
    {
        return glfwGetKey(p_window, glfw_key_code) == GLFW_PRESS;
    }

    bool Keyboard::get_action(const std::string& name) const
    {
        return is_key_down((uint32_t)action_map.at(name));
    }
}