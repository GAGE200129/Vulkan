#pragma once

namespace gage::gfx
{
    class Graphics;
    
    namespace data
    {
         class Camera;
    }
}

namespace gage::scene
{
    class SceneGraph;
}

struct GLFWwindow;
namespace gage::win
{
    class Window;
    class ImguiWindow
    {
    public:
        struct Stats
        {
            float frame_time{};
            float imgui_frame_time{};
        } stats;
    public:
        ImguiWindow(gfx::Graphics &gfx);
        ~ImguiWindow();

        void draw(gfx::data::Camera &camera, Window &window, scene::SceneGraph& scene);

        void clear();
        void end_frame();



    private:
        GLFWwindow *p_window{};
    };
};