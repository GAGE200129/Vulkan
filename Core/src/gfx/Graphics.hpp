#pragma once

#include <stack>
#include <vk_mem_alloc.h>
#include <vector>
#include <functional>
#include <optional>

#include "Exception.hpp"
#include "data/Swapchain.hpp"

namespace vkb
{
    template <typename T>
    class Result;
}

namespace gage::gfx::bind
{
    class IBindable;
}

namespace gage::gfx::data
{
    class Camera;
    class GPUBuffer;
    class CPUBuffer;
    class DescriptorSet;
    class Image;
    class PBRPipeline;
    class ShadowPipeline;
    class GBuffer;
    class FinalAmbient;
}

struct GLFWwindow;
namespace gage::gfx
{
    class Graphics
    {
    public: 
        //Constants
        static constexpr int FRAMES_IN_FLIGHT = 2;

        const std::vector<const char*> ENABLED_INSTANCE_EXTENSIONS
        {
            
        };
        const std::vector<const char*> ENABLED_DEVICE_EXTENSIONS 
        {
            
        };
    private:
        class GlobalUniform;
        friend class bind::IBindable;
        friend class data::Swapchain;
        friend class data::PBRPipeline;
        friend class data::ShadowPipeline;
        friend class data::GPUBuffer;
        friend class data::CPUBuffer;
        friend class data::DescriptorSet;
        friend class data::Image;
        friend class data::GBuffer;
        friend class data::FinalAmbient;
        
    public:
        Graphics(GLFWwindow *window, std::string app_name);
        Graphics(const Graphics &) = delete;
        void operator=(const Graphics &) = delete;
        ~Graphics();

        void wait();
        VkCommandBuffer clear(const data::Camera &camera);
        void end_frame(VkCommandBuffer cmd);


        const std::string &get_app_name() const noexcept;

        //Swapchain size
        void resize(int width, int height);
        void set_resolution_scale(float scale);

        //Shadowmap size
        void resize_shadow_map(uint32_t shadow_map_size);

        const glm::mat4& get_projection() const;
        const glm::mat4& get_view() const;
        const data::Swapchain& get_swapchain() const;
        const data::GBuffer& get_g_buffer() const;
        const data::PBRPipeline& get_pbr_pipeline() const;
        const data::FinalAmbient& get_final_ambient() const;
        GlobalUniform& get_global_uniform();
        VkExtent2D get_scaled_draw_extent();


    private:
        glm::mat4x4 calculate_directional_light_proj_view(const data::Camera& camera, float near, float far);
    private:
        std::mutex uploading_mutex{};
        std::string app_name{};
        std::stack<std::function<void()>> delete_stack{};

        
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};
        VkSurfaceKHR surface{};
        VkDevice device{};
        VkPhysicalDevice physical_device{};


        VkExtent2D draw_extent{};
        VkExtent2D draw_extent_temp{};
        float draw_extent_scale{1.0f};
        bool resize_requested{};

        uint32_t swapchain_image_index{};

        std::optional<data::Swapchain> swapchain{};
        
        VkDescriptorPool desc_pool{};
        VkDescriptorSetLayout global_set_layout{};

        VkCommandPool cmd_pool{};
        
        VkQueue queue{};
        uint32_t queue_family{};

        
        struct FrameData
        {
            VkSemaphore present_semaphore{};
            VkSemaphore render_semaphore{};
            VkFence render_fence{};
            VkCommandBuffer cmd{};
            VkDescriptorSet global_set{};
            VkBuffer global_buffer{};
            VmaAllocation global_alloc{};
            VmaAllocationInfo global_alloc_info{};
        } frame_datas[FRAMES_IN_FLIGHT] {};

        struct DirectionalLight
        {
            glm::vec3 direction{0, -1, 0}; float _padding;
            glm::vec3 color{1, 1, 1}; float _padding2;
        };
        static constexpr uint32_t CASCADE_COUNT{3};
        struct GlobalUniform
        {
            glm::mat4x4 projection{};
            glm::mat4x4 view{};
            glm::vec3 camera_position{}; float _padding{};
            DirectionalLight directional_light{};
            glm::mat4x4 directional_light_proj_views[CASCADE_COUNT]{};
            glm::vec4  directional_light_cascade_planes[CASCADE_COUNT]{ {10, 0, 0, 0}, {30, 0, 0, 0}, {50, 0, 0, 0} };
        } global_uniform{};
        uint32_t frame_index{};

        VmaAllocator allocator{};

        //Pipelines
        uint32_t directional_light_shadow_map_resolution{2048};
        uint32_t directional_light_shadow_map_resolution_temp{2048};
        float directional_light_shadow_map_distance{50.0f};
        bool directional_light_shadow_map_resize_requested{};
        std::unique_ptr<data::GBuffer> g_buffer{};
        std::unique_ptr<data::PBRPipeline> pbr_pipeline{};
        std::unique_ptr<data::FinalAmbient> final_ambient{};
        
    };
}