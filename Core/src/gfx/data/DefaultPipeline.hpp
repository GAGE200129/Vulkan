#pragma once

#include <vk_mem_alloc.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class DefaultPipeline
    {
    public:
        struct DirectionalLight
        {
            glm::vec3 direction{0, -1, 0}; float _padding;
            glm::vec3 color{1, 1, 1}; float _padding2;
        };
        struct GlobalUniform
        {
            glm::mat4x4 projection{};
            glm::mat4x4 view{};
            glm::vec3 camera_position{}; float _padding{};
            DirectionalLight directional_light{};
        } ubo;
    public:
        DefaultPipeline(Graphics& gfx);
        ~DefaultPipeline();


        void begin(VkCommandBuffer cmd);
        void end(VkCommandBuffer cmd);
        void set_push_constant(VkCommandBuffer cmd, const glm::mat4x4& transform);

        VkPipelineLayout get_pipeline_layout() const;
        VkDescriptorSet allocate_instance_set(size_t size_in_bytes, VkBuffer buffer,
           VkImageView albedo_view, VkSampler albedo_sampler,
           VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler) const;
        void free_instance_set(VkDescriptorSet set) const;

        void reset_pipeline();

        VkImage get_color_image_handle() const;
    private:
        void create_default_image_sampler();
        void destroy_default_image_sampler();
        void create_global_uniform_buffer();
        void destroy_global_uniform_buffer();
        void create_pipeline();
        void destroy_pipeline();
        void create_pipeline_layout();
        void destroy_pipeline_layout();
        void create_render_pass();
        void destroy_render_pass();
    private:
        static constexpr VkFormat COLOR_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM};
        static constexpr VkFormat DEPTH_FORMAT = {VK_FORMAT_D32_SFLOAT};

        Graphics& gfx;
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout global_set_layout{};
        VkDescriptorSet global_set{};
        VkDescriptorSetLayout instance_set_layout{};

        VkBuffer global_buffer{};
        VmaAllocation global_alloc{};
        VmaAllocationInfo global_alloc_info{};

        VkImage default_image{};
        VkImageView default_image_view{};
        VmaAllocation default_image_alloc{};
        VkSampler default_sampler{};

        VkRenderPass render_pass{};
        VkFramebuffer frame_buffer{};
        VkDeviceMemory depth_image_memory{};
        VkImage depth_image{};
        VkImageView depth_image_view{};

        VkDeviceMemory color_image_memory{};
        VkImage color_image{};
        VkImageView color_image_view{};

    };
}