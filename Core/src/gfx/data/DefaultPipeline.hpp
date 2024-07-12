#pragma once
#include "../Graphics.hpp"

#include "ShadowPipeline.hpp"

namespace gage::gfx::data
{
    class DefaultPipeline
    {
    public:
        DefaultPipeline(Graphics& gfx);
        ~DefaultPipeline();

        void begin_shadow(VkCommandBuffer cmd);
        void end_shadow(VkCommandBuffer cmd);

        void begin(VkCommandBuffer cmd);
        void end(VkCommandBuffer cmd);

        VkCommandBuffer begin_cmd();
        void end_cmd(VkCommandBuffer cmd);
        void set_push_constant(VkCommandBuffer cmd, const glm::mat4x4& transform);

        VkPipelineLayout get_pipeline_layout() const;
        VkPipelineLayout get_shadow_pipeline_layout() const;
        VkDescriptorSet allocate_instance_set(size_t size_in_bytes, VkBuffer buffer,
           VkImageView albedo_view, VkSampler albedo_sampler,
           VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler,
           VkImageView normal_view, VkSampler normal_sampler) const;
        void free_instance_set(VkDescriptorSet set) const;

        void reset_pipeline();

        VkImage get_color_image_handle() const;
        VkSemaphore get_render_finished_semaphore(uint32_t i) const;
    private:
        void create_default_image_sampler();
        void destroy_default_image_sampler();
        void create_pipeline();
        void destroy_pipeline();
        void create_pipeline_layout();
        void destroy_pipeline_layout();
        void create_render_pass();
        void destroy_render_pass();

        void allocate_cmd();
        void free_cmd();
    private:
        static constexpr VkFormat COLOR_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM};
        static constexpr VkFormat DEPTH_FORMAT = {VK_FORMAT_D32_SFLOAT};

        Graphics& gfx;
        ShadowPipeline shadow_pipeline;

        VkCommandBuffer cmds[Graphics::FRAMES_IN_FLIGHT]{};
        VkSemaphore render_finished_semaphores[Graphics::FRAMES_IN_FLIGHT]{};

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

        VkDeviceMemory color_image_memory{};
        VkImage color_image{};
        VkImageView color_image_view{};
    };
}