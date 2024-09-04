#pragma once

#include <vulkan/vulkan.h>

#include "MainPass.hpp"
#include "ShadowPass.hpp"
#include "LightPass.hpp"
#include "SSAOPass.hpp"

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::g_buffer
{
    class GBuffer
    {
    public:
        GBuffer(const Graphics& gfx);
        ~GBuffer();

        void begin_shadowpass(VkCommandBuffer cmd) const;
        void begin_mainpass(VkCommandBuffer cmd) const;
        void begin_lightpass(VkCommandBuffer cmd) const;
        void begin_ssaopass(VkCommandBuffer cmd) const;
        void end(VkCommandBuffer cmd) const;

        VkRenderPass get_mainpass_render_pass() const;
        VkRenderPass get_lightpass_render_pass() const;
        VkRenderPass get_shadowpass_render_pass() const;
        VkRenderPass get_ssao_render_pass() const;

        VkImageView get_depth_view() const;
        VkImageView get_stencil_view() const;
        VkImageView get_normal_view() const;
        VkImageView get_albedo_view() const;
        VkImageView get_mr_view() const;
        VkImageView get_ssao_view() const;
        VkImage get_final_color() const;

        void reset();
        void reset_shadowmap();

    private:
        const Graphics& gfx;
    public:
        MainPass main_pass;
        ShadowPass shadow_pass;
        LightPass light_pass;
        SSAOPass ssao_pass;
    };
}