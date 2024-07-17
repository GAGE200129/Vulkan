#pragma once

#include <vulkan/vulkan.h>
#include <glm/vec3.hpp>

namespace gage::gfx
{
    class Graphics;
}


namespace gage::gfx::data
{
    class PointLight
    {  
    public:
        struct Data
        {
            glm::vec3 position{0, 0, 0}; float intensity{1.0f};
            glm::vec3 color{1, 1, 1};

            float constant{1.0};
            float linear{0.35};
            float exponent{0.44};
        };
    public:
        PointLight(Graphics& gfx);
        ~PointLight();

        void process(VkCommandBuffer cmd, const Data& data) const;

        void reset();

        VkPipelineLayout get_layout() const;
    private:
        void link_desc_to_g_buffer();
    private:
        Graphics& gfx;
        VkSampler default_sampler{};
        VkDescriptorSetLayout desc_layout{};
        VkDescriptorSet desc{};
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
    };
}