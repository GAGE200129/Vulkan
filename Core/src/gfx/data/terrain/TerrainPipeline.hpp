#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::terrain
{
    class TerrainPipeline
    {
    public:
        TerrainPipeline(Graphics& gfx);
        ~TerrainPipeline();

        void bind(VkCommandBuffer cmd) const;
 
        //void set_push_constant(VkCommandBuffer cmd,  const glm::mat4x4& transform);

        VkPipelineLayout get_layout() const;
        void reset();
    private:
        void create_pipeline();
        void destroy_pipeline();
        void create_pipeline_layout();
        void destroy_pipeline_layout();

    private:

        Graphics& gfx;

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout desc_layout{};
    };
}