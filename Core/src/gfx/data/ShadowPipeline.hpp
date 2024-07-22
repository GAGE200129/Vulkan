#pragma once


namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class Camera;
    class ShadowPipeline
    {

    public:
        ShadowPipeline(Graphics& gfx);
        ~ShadowPipeline();


        void bind(VkCommandBuffer cmd) const;

        VkPipelineLayout get_layout() const;
    private:
        void create_pipeline_layout();
        void destroy_pipeline_layout();

        void create_pipeline();
        void destroy_pipeline();
    private:

        Graphics& gfx;
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout desc_layout{};

    };
}