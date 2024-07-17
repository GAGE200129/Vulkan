#pragma once


namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class PBRPipeline
    {
    public:
        PBRPipeline(Graphics& gfx);
        ~PBRPipeline();

        void bind(VkCommandBuffer cmd) const;

        void set_push_constant(VkCommandBuffer cmd, const glm::mat4x4& transform);

        VkPipelineLayout get_layout() const;
        VkDescriptorSet allocate_instance_set(size_t size_in_bytes, VkBuffer buffer,
           VkImageView albedo_view, VkSampler albedo_sampler,
           VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler,
           VkImageView normal_view, VkSampler normal_sampler) const;
        void free_instance_set(VkDescriptorSet set) const;

        void reset();


    private:
        void create_default_image_sampler();
        void destroy_default_image_sampler();
        void create_pipeline();
        void destroy_pipeline();
        void create_pipeline_layout();
        void destroy_pipeline_layout();

    private:

        Graphics& gfx;

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout set_layout{};

        VkImage default_image{};
        VkImageView default_image_view{};
        VmaAllocation default_image_alloc{};
        VkSampler default_sampler{};
    };
}