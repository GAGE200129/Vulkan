#pragma once

#include <cstdint>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::utils
{
    class Camera;
}

struct GLFWwindow;
namespace gage::win
{
    class Window;
    class ImguiWindow
    {
    public:
        ImguiWindow(gfx::Graphics &gfx);
        ~ImguiWindow();

        void draw(utils::Camera &camera, Window &window);

        void clear();
        void end_frame();

    private:
        GLFWwindow *p_window{};
        uint32_t gfx_color_texture_mem{};
        uint32_t gfx_color_texture{};

        uint32_t gfx_depth_texture_mem{};
        uint32_t gfx_depth_texture{};
    };
};