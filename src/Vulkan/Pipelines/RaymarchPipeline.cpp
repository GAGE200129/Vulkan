#include "pch.hpp"

#include "../VulkanEngine.hpp"

bool VulkanEngine::raymarchPipelineInit()
{
    spdlog::info("Raymarch pipeline init.");

    // Descriptor layout

    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    vk::DescriptorSetLayoutCreateInfo layoutCI;
    layoutCI.setBindings(layoutBinding);
    
    auto[descriptorLayoutResult, descriptorLayout] = gData.device.createDescriptorSetLayout(layoutCI);
    if(descriptorLayoutResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create descriptor layout: {}", vk::to_string(descriptorLayoutResult));
        return false;
    }

    // Pipeline
    std::vector<char> vertexCode, fragmentCode;
    Utils::filePathToVectorOfChar("res/shaders/raymarch.vert.spv", vertexCode);
    Utils::filePathToVectorOfChar("res/shaders/raymarch.frag.spv", fragmentCode);

    vk::ShaderModule vertexModule, fragmentModule;
    if(!VulkanEngine::initShaderModule(vertexCode, vertexModule))
        return false;
    if(!VulkanEngine::initShaderModule(fragmentCode, fragmentModule))
        return false;
    vk::PipelineShaderStageCreateInfo vertStageCI, fragStageCI;
    vertStageCI.setStage(vk::ShaderStageFlagBits::eVertex)
        .setPName("main")
        .setModule(vertexModule);
    fragStageCI.setStage(vk::ShaderStageFlagBits::eFragment)
        .setPName("main")
        .setModule(fragmentModule);

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {vertStageCI, fragStageCI};
    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateCI;
    dynamicStateCI.setDynamicStates(dynamicStates);
    vk::PipelineVertexInputStateCreateInfo vertexInputCI;

    // Position, normal, uv
    std::array<vk::VertexInputBindingDescription, 1> vertexInputBindingDescriptions;
    vertexInputBindingDescriptions[0].setBinding(0).setStride(sizeof(glm::vec3)).setInputRate(vk::VertexInputRate::eVertex);

    std::array<vk::VertexInputAttributeDescription, 1> vertexInputAttributeDescription;
    vertexInputAttributeDescription[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat);

    vertexInputCI.setVertexBindingDescriptions(vertexInputBindingDescriptions)
        .setVertexAttributeDescriptions(vertexInputAttributeDescription);
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCI;
    inputAssemblyCI.setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(false);
    vk::Viewport viewport;
    viewport.setX(0)
        .setY(0)
        .setWidth(gData.swapExtent.width)
        .setHeight(gData.swapExtent.height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    vk::Rect2D scissor;
    scissor.setOffset({0, 0})
        .setExtent(gData.swapExtent);
    vk::PipelineViewportStateCreateInfo viewportStateCI;
    viewportStateCI.setViewports(viewport)
        .setScissors(scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCI;
    rasterizationStateCI.setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eNone)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(false)
        .setDepthBiasConstantFactor(0.0f)
        .setDepthBiasSlopeFactor(0.0f)
        .setDepthBiasClamp(0.0f);

    vk::PipelineMultisampleStateCreateInfo multisampleStateCI;
    multisampleStateCI.setSampleShadingEnable(false)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(false)
        .setAlphaToOneEnable(false);

    vk::PipelineColorBlendAttachmentState blendAttachmentState;
    blendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eA |
                                           vk::ColorComponentFlagBits::eR |
                                           vk::ColorComponentFlagBits::eG |
                                           vk::ColorComponentFlagBits::eB)
        .setBlendEnable(true)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrc1Alpha)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrc1Alpha)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    vk::PipelineColorBlendStateCreateInfo colorBlendingCI;
    colorBlendingCI.setLogicOp(vk::LogicOp::eCopy)
        .setLogicOpEnable(true)
        .setAttachments(blendAttachmentState)
        .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil
        .setDepthTestEnable(true)
        .setDepthWriteEnable(true)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(false)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(false);

    vk::PipelineLayoutCreateInfo pipelineLayoutCI;
    auto layouts = std::array{gData.globalDescriptorLayout};
    pipelineLayoutCI.setSetLayouts(layouts);

    auto[layoutResult, layout] = gData.device.createPipelineLayout(pipelineLayoutCI);
    if(layoutResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create pipeline layout: {}", vk::to_string(layoutResult));
        return false;
    }
    

    vk::GraphicsPipelineCreateInfo graphicsPipelineCI;
    graphicsPipelineCI.setStages(shaderStages)
        .setPVertexInputState(&vertexInputCI)
        .setPInputAssemblyState(&inputAssemblyCI)
        .setPViewportState(&viewportStateCI)
        .setPRasterizationState(&rasterizationStateCI)
        .setPMultisampleState(&multisampleStateCI)
        .setPDepthStencilState(&depthStencil)
        .setPColorBlendState(&colorBlendingCI)
        .setPDynamicState(&dynamicStateCI)
        .setLayout(layout)
        .setRenderPass(gData.renderPass)
        .setSubpass(0)
        .setBasePipelineHandle(nullptr)
        .setBasePipelineIndex(-1);

    auto graphicsPipelineResult = gData.device.createGraphicsPipeline(nullptr, graphicsPipelineCI);
    if (graphicsPipelineResult.result != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create graphics pipeline !");
        return false;
    }
    gData.raymarchPipeline.pipeline = graphicsPipelineResult.value;
    gData.raymarchPipeline.layout = layout;
    gData.raymarchPipeline.descriptorLayout = descriptorLayout;

    gData.device.destroyShaderModule(vertexModule);
    gData.device.destroyShaderModule(fragmentModule);

    return true;
}

void VulkanEngine::raymarchPipelineCleanup()
{
    spdlog::info("Raymarch pipeline cleanup.");
    gData.device.destroyPipeline(gData.raymarchPipeline.pipeline);
    gData.device.destroyPipelineLayout(gData.raymarchPipeline.layout);
    gData.device.destroyDescriptorSetLayout(gData.raymarchPipeline.descriptorLayout);
}