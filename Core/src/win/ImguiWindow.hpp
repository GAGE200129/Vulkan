#pragma once


struct GLFWwindow;
namespace gage::win
{
    class ImguiWindow
    {
    public:
        ImguiWindow();
        ~ImguiWindow();

        void clear();
        void end_frame();
    private:
         GLFWwindow* p_window{};
    };
};