#pragma once

#include <vector>
#include <glm/vec3.hpp>

#include "Image.hpp"
#include "GPUBuffer.hpp"

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class SSAO
    {
    public:
        SSAO(Graphics& gfx);
        ~SSAO();

        void process(VkCommandBuffer cmd) const;

        void reset();
        void link_desc_to_g_buffer();
        void link_desc_to_kernel_buffer();
    private:
        void generate_kernel_and_noises();
        void create_pipeline();
        
    private:
        static constexpr size_t KERNEL_SIZE = 8;
        struct PushConstantFragment
        {
            float radius;
            float bias;
            glm::vec2 noise_scale;
            float resolution_scale;
        };
        Graphics& gfx;
        std::vector<glm::vec4> kernel{};
        std::vector<glm::vec3> noises{};
        std::unique_ptr<Image> image{};
        std::unique_ptr<GPUBuffer> kernel_buffer{};

        VkDescriptorSetLayout desc_layout{};
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSet desc{};

    };
}