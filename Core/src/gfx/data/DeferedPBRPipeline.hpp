#pragma once
#include "../Graphics.hpp"

#include "ShadowPipeline.hpp"
#include "PostprocessPipeline.hpp"

namespace gage::gfx::data
{
    class DeferedPBRPipeline
    {
    public:
        DeferedPBRPipeline(Graphics& gfx);
        ~DeferedPBRPipeline();

        void begin(VkCommandBuffer cmd) const;
        void end(VkCommandBuffer cmd) const;

        void set_push_constant(VkCommandBuffer cmd, const glm::mat4x4& transform);

        VkPipelineLayout get_layout() const;
        VkDescriptorSet allocate_instance_set(size_t size_in_bytes, VkBuffer buffer,
           VkImageView albedo_view, VkSampler albedo_sampler,
           VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler,
           VkImageView normal_view, VkSampler normal_sampler) const;
        void free_instance_set(VkDescriptorSet set) const;

        void reset();

        VkSampler get_default_sampler() const;
        VkImage get_position() const;
        VkImage get_normal() const;
        VkImage get_uv() const;


        ShadowPipeline& get_shadow_pipeline();
        const ShadowPipeline& get_shadow_pipeline() const;
    private:
        void create_default_image_sampler();
        void destroy_default_image_sampler();
        void create_pipeline();
        void destroy_pipeline();
        void create_pipeline_layout();
        void destroy_pipeline_layout();
        void create_render_pass();
        void destroy_render_pass();

        void create_g_buffer_images();
        void destroy_g_buffer_images();

    private:
        static constexpr VkFormat POSITION_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat NORMAL_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat UV_FORMAT = {VK_FORMAT_R16G16_SFLOAT};
        static constexpr VkFormat DEPTH_FORMAT = {VK_FORMAT_D32_SFLOAT};

        Graphics& gfx;
        ShadowPipeline shadow_pipeline;
        PostprocessPipeline post_pipeline;

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout instance_set_layout{};

        VkImage default_image{};
        VkImageView default_image_view{};
        VmaAllocation default_image_alloc{};
        VkSampler default_sampler{};

        //Main pass
        VkRenderPass render_pass{};
        VkFramebuffer frame_buffer{};
        VkDeviceMemory depth_image_memory{};
        VkImage depth_image{};
        VkImageView depth_image_view{};

        VkDeviceMemory position_memory{};
        VkImage position{};
        VkImageView position_view{};

        VkDeviceMemory normal_memory{};
        VkImage normal{};
        VkImageView normal_view{};

        VkDeviceMemory uv_memory{};
        VkImage uv{};
        VkImageView uv_view{};
    };
}