#include "pch.hpp"
#include "VulkanEngine.hpp"

bool VulkanEngine::initRenderPass()
{
    vk::AttachmentDescription colorAttachment;
    colorAttachment.setFormat(gData.surfaceFormat.format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription depthAttachment;
    depthAttachment.setFormat(vk::Format::eD32Sfloat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.setAttachment(1)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachments(colorAttachmentRef)
        .setPDepthStencilAttachment(&depthAttachmentRef);

    vk::SubpassDependency subpassDependency;
    subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    vk::RenderPassCreateInfo renderPassCI;
    auto attachments = std::array{colorAttachment, depthAttachment};
    renderPassCI.setAttachments(attachments)
        .setSubpasses(subpass)
        .setDependencies(subpassDependency);

    auto [renderPassResult, renderPass] = gData.device.createRenderPass(renderPassCI);
    if (renderPassResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create render pass: {}", vk::to_string(renderPassResult));
        return false;
    }
    gData.renderPass = renderPass;
    return true;
}