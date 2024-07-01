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

        VkPipelineLayout get_pipeline_layout() const;
        VkDescriptorSetLayout get_instance_set_layout() const;
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
    };
}