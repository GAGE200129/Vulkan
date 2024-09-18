#include <pch.hpp>
#include "Keyboard.hpp"

#include "hid.hpp"

namespace gage::hid
{
    static std::bitset<512> g_keys_down{};
    static std::bitset<512> g_prev_keys_down{};
    static void key_callback_fn(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if(action == GLFW_PRESS)
        {
            g_keys_down[key] = true;
        }
        else if(action == GLFW_RELEASE)
        {
            g_keys_down[key] = false;
        }
    }
    Keyboard::Keyboard(GLFWwindow* p_window) :
        p_window(p_window)
    {
        glfwSetKeyCallback(p_window, key_callback_fn);
        
    }

    Keyboard::~Keyboard() {}

    void Keyboard::update()
    {
        g_prev_keys_down = g_keys_down;
    }

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
        return g_keys_down[glfw_key_code];
    }

    bool Keyboard::get_action(const std::string& name) const
    {
        return g_keys_down[(uint32_t)action_map.at(name)];
    }

    bool Keyboard::get_action_once(const std::string& name) const
    {
        uint32_t key = (uint32_t)action_map.at(name);
        return g_keys_down[key] && !g_prev_keys_down[key];
    }
}