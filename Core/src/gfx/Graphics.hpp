#pragma once

#include <vector>


#include "Exception.hpp"
#include "data/Instance.hpp"
#include "data/Device.hpp"
#include "data/Swapchain.hpp"
#include "data/DescriptorPool.hpp"
#include "data/CommandPool.hpp"
#include "data/FrameData.hpp"
#include "data/Allocator.hpp"
#include "data/GlobalDescriptorSetLayout.hpp"
#include "data/g_buffer/GBuffer.hpp"
#include "data/Camera.hpp"
#include "data/AmbientLight.hpp"
#include "data/DirectionalLight.hpp"
#include "data/PointLight.hpp"
#include "data/SSAO.hpp"
#include "data/Swapchain.hpp"
#include "data/Default.hpp"

namespace gage::gfx::data
{
    class Camera;

}

struct GLFWwindow;
namespace gage::gfx
{
    class Graphics
    {
    public:
        // Constants
        static constexpr int FRAMES_IN_FLIGHT = 2;
        static constexpr uint32_t CASCADE_COUNT{3};

        static const std::vector<const char *> ENABLED_INSTANCE_EXTENSIONS;
        static const std::vector<const char *> ENABLED_DEVICE_EXTENSIONS;

        struct GlobalUniform
        {
            glm::mat4x4 projection{};
            glm::mat4x4 inv_projection{};
            glm::mat4x4 view{};
            glm::mat4x4 inv_view{};
            glm::vec3 camera_position{};
            float padding;

            glm::vec3 ambient_light_color{1, 1, 1};
            float ambient_light_intensity{0.1f};
            float ambient_fog_begin{30.1f};
            float ambient_fog_end{500.1f};
            float padding4[2];

            glm::vec3 directional_light_direction{0, -1, 0};
            float padding2;
            glm::vec3 directional_light_color{1, 1, 1};
            float padding3;

            glm::mat4x4 directional_light_proj_views[CASCADE_COUNT]{};
            glm::vec4 directional_light_cascade_planes[CASCADE_COUNT]{{10, 0, 0, 0}, {30, 0, 0, 0}, {50, 0, 0, 0}};
        };
    public:
        Graphics(GLFWwindow *window, uint32_t width, uint32_t height, std::string app_name);
        Graphics(const Graphics &) = delete;
        void operator=(const Graphics &) = delete;
        Graphics(Graphics&&) = delete;
        Graphics& operator=(Graphics&&) = delete;
        ~Graphics();

        void wait();
        VkCommandBuffer clear(const data::Camera &camera);
        void end_frame(VkCommandBuffer cmd);



        void resize(int width, int height);

        // Shadowmap size
        void resize_shadow_map(uint32_t shadow_map_size);

        VkExtent2D get_scaled_draw_extent() const;

    private:
        glm::mat4x4 calculate_directional_light_proj_view(const data::Camera &camera, float near, float far);

    public:
        std::string app_name{};
        VkExtent2D draw_extent{};
        VkExtent2D draw_extent_temp{};
        data::Instance instance;
        data::Device device;
        data::Swapchain swapchain;
        data::DescriptorPool desc_pool;
        data::GlobalDescriptorSetLayout global_desc_layout;
        data::CommandPool cmd_pool;
        data::Allocator allocator;
        data::Default defaults;
        data::FrameData frame_datas[FRAMES_IN_FLIGHT];

        uint32_t directional_light_shadow_map_resolution;
        uint32_t directional_light_shadow_map_resolution_temp;

        data::g_buffer::GBuffer geometry_buffer;
        data::AmbientLight final_ambient;
        data::DirectionalLight directional_light;
        data::PointLight point_light;
        data::SSAO ssao;
        
        
        

        uint32_t frame_index{};

        GlobalUniform global_uniform{};
        float draw_extent_scale{1.0f};

    };
}