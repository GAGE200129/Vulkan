#pragma once

#include <cstdint>

namespace gage::gfx
{
    class Graphics;
    
    namespace data
    {
         class Camera;
    }
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

        void draw(gfx::data::Camera &camera, Window &window);

        void clear();
        void end_frame();
    private:
        //void create_viewport(gfx::Graphics& gfx);
        //void destroy_viewport();
    private:
        GLFWwindow *p_window{};
       //uint32_t gfx_color_texture_mem{};
       //uint32_t gfx_color_texture{};

       //uint32_t gfx_depth_texture_mem{};
       //uint32_t gfx_depth_texture{};
    };
};