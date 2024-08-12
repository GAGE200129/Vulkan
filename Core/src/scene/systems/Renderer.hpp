#pragma once

#include "../components/MeshRenderer.hpp"

#include <vector>
#include <memory>
#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/CPUBuffer.hpp>

namespace gage::gfx::data
{
    class Camera;
}

namespace gage::scene
{
    class SceneGraph;
}

namespace gage::scene::systems
{
    class Renderer
    {
        friend class scene::SceneGraph;

    public:
        struct MaterialSetAllocInfo
        {
            size_t size_in_bytes{};
            VkBuffer buffer{};
            VkImageView albedo_view{};
            VkSampler albedo_sampler{};
            VkImageView metalic_roughness_view{};
            VkSampler metalic_roughness_sampler{};
            VkImageView normal_view{};
            VkSampler normal_sampler{};
        };

        struct MeshRenderer
        {
            std::unique_ptr<gfx::data::CPUBuffer> animation_buffers[gfx::Graphics::FRAMES_IN_FLIGHT]{};
            VkDescriptorSet animation_descs[gfx::Graphics::FRAMES_IN_FLIGHT]{};

            std::unique_ptr<components::MeshRenderer> mesh_renderer;
        };

    public:
        Renderer(const gfx::Graphics &gfx);
        ~Renderer();

        void init();
        void shutdown();

        void render_depth(VkCommandBuffer cmd) const;
        void render(VkCommandBuffer cmd) const;

        void add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer> mesh_renderer);

        VkDescriptorSet allocate_material_set(const MaterialSetAllocInfo &info) const;
        VkDescriptorSet allocate_animation_set(size_t size_in_bytes, VkBuffer buffer) const;

    private:
        void create_pipeline();
        void create_depth_pipeline();

    private:
        static constexpr uint8_t STENCIL_VALUE = 0x01;
        const gfx::Graphics &gfx;
        std::vector<MeshRenderer> mesh_renderers;

        VkDescriptorSetLayout material_set_layout{};
        VkDescriptorSetLayout animation_set_layout{};

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};

        // Shadow map
        VkPipelineLayout depth_pipeline_layout{};
        VkPipeline depth_pipeline{};
        // VkDescriptorSetLayout depth_desc_layout{};
    };
}