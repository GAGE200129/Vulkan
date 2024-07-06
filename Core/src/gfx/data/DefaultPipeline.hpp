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
        struct GlobalUniform
        {
            glm::mat4x4 projection{};
            glm::mat4x4 view{};
            glm::vec3 camera_position; float _padding{};
            glm::vec3 point_light_position{}; float _padding2{};
            glm::vec4 ambient{0.15f, 0.15f, 0.15f, 1.0f};
            glm::vec4 diffuse_color{1.0f,1.0f, 1.0f, 1.0f};
            float diffuse_intensity{1.0f};
            float att_constant{1.0f};
            float att_linear{0.045f};
            float att_exponent{0.0075f};
        } ubo;
    public:
        DefaultPipeline(Graphics& gfx);
        ~DefaultPipeline();

        void bind(VkCommandBuffer cmd);
        void set_push_constant(VkCommandBuffer cmd, const glm::mat4x4& transform);

        VkPipelineLayout get_pipeline_layout() const;
        VkDescriptorSet allocate_instance_set(size_t size_in_bytes, VkBuffer buffer,
           VkImageView albedo_view, VkSampler albedo_sampler,
           VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler) const;
        void free_instance_set(VkDescriptorSet set) const;

        void reset_pipeline();
    private:
        void create_default_image_sampler();
        void destroy_default_image_sampler();
        void create_global_uniform_buffer();
        void destroy_global_uniform_buffer();
        void create_pipeline();
        void destroy_pipeline();
        void create_pipeline_layout();
        void destroy_pipeline_layout();
    private:

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
    };
}