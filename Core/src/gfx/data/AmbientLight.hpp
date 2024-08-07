#pragma once

#include <vk_mem_alloc.h>

namespace gage::gfx
{
    class Graphics;
}


namespace gage::gfx::data
{
    class AmbientLight
    {   
        struct FragmentPushConstant
        {
            float time{0.0f};
            float fbm_scale{0.9};
            float fbm_factor{2.0};
            float height{1.0f};
        };
    public:
        AmbientLight(Graphics& gfx);
        ~AmbientLight();

        void update(float delta);

        void process(VkCommandBuffer cmd) const;

        void reset();

        VkPipelineLayout get_layout() const;
    private:
        void link_desc_to_g_buffer();
    private:
        Graphics& gfx;
        VkDescriptorSetLayout desc_layout{};
        VkDescriptorSet desc{};
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};



        float time_scale{0.1f};
    public:
        FragmentPushConstant fs_ps{};
    };
}