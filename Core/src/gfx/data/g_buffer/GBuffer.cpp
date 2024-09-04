#include <pch.hpp>
#include "GBuffer.hpp"

#include "../../Graphics.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/utils/VulkanHelper.hpp>

namespace gage::gfx::data::g_buffer
{
    GBuffer::GBuffer(const Graphics &gfx) : 
        gfx(gfx),
        main_pass(gfx),
        shadow_pass(gfx),
        light_pass(gfx),
        ssao_pass(gfx)
    {
    }
    GBuffer::~GBuffer()
    {
    }

    void GBuffer::reset()
    {
        main_pass.reset();
        light_pass.reset();
        ssao_pass.reset();
    }

    void GBuffer::reset_shadowmap()
    {
        shadow_pass.reset();
    }

    void GBuffer::begin_shadowpass(VkCommandBuffer cmd) const
    {
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = shadow_pass.shadowpass_renderpass;
        render_pass_begin_info.framebuffer = shadow_pass.shadowpass_framebuffer;
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = {gfx.directional_light_shadow_map_resolution, gfx.directional_light_shadow_map_resolution};
        std::array<VkClearValue, 1> clear_values{};
        clear_values[0].depthStencil = {1.0, 0};
        render_pass_begin_info.clearValueCount = clear_values.size();
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GBuffer::begin_mainpass(VkCommandBuffer cmd) const
    {
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = main_pass.render_pass;
        render_pass_begin_info.framebuffer = main_pass.framebuffer;
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = gfx.get_scaled_draw_extent();
        std::array<VkClearValue, 4> clear_values{};
        //clear_values[0].color = {{0.0f, 0.0f, 0.0f}};
        clear_values[0].color = {{0.0f, 0.0f, 0.0f}};
        clear_values[1].color = {{0.0f, 0.0f, 0.0f}};
        clear_values[2].color = {{0.0f, 0.0f, 0.0f}};
        clear_values[3].depthStencil = {1.0f, 0};
        render_pass_begin_info.clearValueCount = clear_values.size();
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GBuffer::begin_lightpass(VkCommandBuffer cmd) const
    {
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = light_pass.finalpass_renderpass;
        render_pass_begin_info.framebuffer = light_pass.finalpass_framebuffer;
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = gfx.get_scaled_draw_extent();
        std::array<VkClearValue, 1> clear_values{};
        clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        render_pass_begin_info.clearValueCount = clear_values.size();
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GBuffer::begin_ssaopass(VkCommandBuffer cmd) const
    {
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = ssao_pass.render_pass;
        render_pass_begin_info.framebuffer = ssao_pass.framebuffer;
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = gfx.get_scaled_draw_extent();
        std::array<VkClearValue, 1> clear_values{};
        clear_values[0].color = {{1.0f, 0.0f, 0.0f, 1.0f}};
        render_pass_begin_info.clearValueCount = clear_values.size();
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GBuffer::end(VkCommandBuffer cmd) const
    {
        vkCmdEndRenderPass(cmd);
    }

    VkRenderPass GBuffer::get_mainpass_render_pass() const
    {
        return main_pass.render_pass;
    }

    VkRenderPass GBuffer::get_lightpass_render_pass() const
    {
        return light_pass.finalpass_renderpass;
    }

    VkRenderPass GBuffer::get_ssao_render_pass() const
    {
        return ssao_pass.render_pass;
    }

    VkRenderPass GBuffer::get_shadowpass_render_pass() const
    {
        return shadow_pass.shadowpass_renderpass;
    }
    VkImageView GBuffer::get_depth_view() const
    {
        return main_pass.depth_image_view;
    }
    VkImageView GBuffer::get_stencil_view() const
    {
        return main_pass.stencil_image_view;
    }
    VkImageView GBuffer::get_normal_view() const
    {
        return main_pass.normal_view;
    }
    VkImageView GBuffer::get_albedo_view() const
    {
        return main_pass.albedo_view;
    }

    VkImageView GBuffer::get_mr_view() const
    {
        return main_pass.mr_view;
    }
    VkImageView GBuffer::get_ssao_view() const
    {
        return ssao_pass.image_view;
    }

    VkImage GBuffer::get_final_color() const
    {
        return light_pass.finalpass_image;
    }

}