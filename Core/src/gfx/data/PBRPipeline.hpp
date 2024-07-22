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
        void bind_depth(VkCommandBuffer cmd) const;

        void set_push_constant(VkCommandBuffer cmd, const glm::mat4x4& transform);

        VkPipelineLayout get_layout() const;
        VkPipelineLayout get_depth_layout() const;
        VkDescriptorSet allocate_material_set(size_t size_in_bytes, VkBuffer buffer,
           VkImageView albedo_view, VkSampler albedo_sampler,
           VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler,
           VkImageView normal_view, VkSampler normal_sampler) const;
        VkDescriptorSet allocate_animation_set(size_t size_in_bytes, VkBuffer buffer) const;
        void free_descriptor_set(VkDescriptorSet set) const;

        void reset();


    private:
        void create_descriptor_set_layouts();
        void destroy_descriptor_set_layouts();

        void create_pipeline();
        void destroy_pipeline();

        void create_depth_pipeline();
        void destroy_depth_pipeline();

    private:

        Graphics& gfx;

        VkDescriptorSetLayout material_set_layout{};
        VkDescriptorSetLayout animation_set_layout{};

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        

        //Shadow map
        VkPipelineLayout depth_pipeline_layout{};
        VkPipeline depth_pipeline{};
        //VkDescriptorSetLayout depth_desc_layout{};
    };
}