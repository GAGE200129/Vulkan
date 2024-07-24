#pragma once

#include <bitset>

struct GLFWwindow;
namespace gage::hid
{
    enum class KeyCodes : uint32_t
    {
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        SPACE = 32,
    };
    class Keyboard
    {
    public:
        Keyboard(GLFWwindow* p_window);
        ~Keyboard();
        
        void register_action(KeyCodes ey_code, const std::string& name);

        bool get_action(const std::string& name) const;
        bool is_key_down(uint32_t glfw_key_code) const;
        

    private:
        GLFWwindow* p_window;
        std::bitset<500> key_pressed;
        std::unordered_map<std::string, KeyCodes> action_map{};
    };
}