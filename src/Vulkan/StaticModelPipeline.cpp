#include "pch.hpp"

#include "VulkanEngine.hpp"

#include "Asset/FileLoader.hpp"

bool VulkanEngine::staticModelPipelineInit()
{
    spdlog::info("Static model pipeline init.");
    // Descriptor layout

    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.setBinding(0)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    vk::DescriptorSetLayoutCreateInfo layoutCI;
    layoutCI.setBindings(layoutBinding);
    
    auto[imageDescriptorResult, imageDescriptor] = VulkanEngine::gData.device.createDescriptorSetLayout(layoutCI);
    if(imageDescriptorResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create image descriptor layout: {}", vk::to_string(imageDescriptorResult));
        return false;
    }
    gData.staticModelPipeline.imageDescriptorLayout = imageDescriptor;
    // Pipeline
    std::vector<char> vertexCode, fragmentCode;
    FileLoader::filePathToVectorOfChar("res/shaders/static_model.vert.spv", vertexCode);
    FileLoader::filePathToVectorOfChar("res/shaders/static_model.frag.spv", fragmentCode);

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
    std::array<vk::VertexInputBindingDescription, 3> vertexInputBindingDescriptions;
    vertexInputBindingDescriptions[0].setBinding(0).setStride(sizeof(glm::vec3)).setInputRate(vk::VertexInputRate::eVertex);
    vertexInputBindingDescriptions[1].setBinding(1).setStride(sizeof(glm::vec3)).setInputRate(vk::VertexInputRate::eVertex);
    vertexInputBindingDescriptions[2].setBinding(2).setStride(sizeof(glm::vec2)).setInputRate(vk::VertexInputRate::eVertex);

    std::array<vk::VertexInputAttributeDescription, 3> vertexInputAttributeDescription;
    vertexInputAttributeDescription[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat);
    vertexInputAttributeDescription[1].setBinding(1).setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat);
    vertexInputAttributeDescription[2].setBinding(2).setLocation(2).setFormat(vk::Format::eR32G32Sfloat);

    vertexInputCI.setVertexBindingDescriptions(vertexInputBindingDescriptions)
        .setVertexAttributeDescriptions(vertexInputAttributeDescription);
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCI;
    inputAssemblyCI.setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(false);
    vk::Viewport viewport;
    viewport.setX(0)
        .setY(0)
        .setWidth(VulkanEngine::gData.swapExtent.width)
        .setHeight(VulkanEngine::gData.swapExtent.height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    vk::Rect2D scissor;
    scissor.setOffset({0, 0})
        .setExtent(VulkanEngine::gData.swapExtent);
    vk::PipelineViewportStateCreateInfo viewportStateCI;
    viewportStateCI.setViewports(viewport)
        .setScissors(scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCI;
    rasterizationStateCI.setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
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
        .setBlendEnable(false)
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eZero)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    vk::PipelineColorBlendStateCreateInfo colorBlendingCI;
    colorBlendingCI.setLogicOp(vk::LogicOp::eCopy)
        .setLogicOpEnable(false)
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
    vk::PushConstantRange pushConstantCI;
    pushConstantCI.setStageFlags(vk::ShaderStageFlagBits::eVertex)
        .setSize(sizeof(glm::mat4x4));

    auto layouts = std::array{VulkanEngine::gData.globalDescriptorLayout, gData.staticModelPipeline.imageDescriptorLayout};
    pipelineLayoutCI.setSetLayouts(layouts)
        .setPushConstantRanges(pushConstantCI);

    auto[layoutResult, layout] = VulkanEngine::gData.device.createPipelineLayout(pipelineLayoutCI);
    if(layoutResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create pipeline layout: {}", vk::to_string(layoutResult));
        return false;
    }
    gData.staticModelPipeline.layout = layout;

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
        .setLayout(gData.staticModelPipeline.layout)
        .setRenderPass(VulkanEngine::gData.renderPass)
        .setSubpass(0)
        .setBasePipelineHandle(nullptr)
        .setBasePipelineIndex(-1);

    auto graphicsPipelineResult = VulkanEngine::gData.device.createGraphicsPipeline(nullptr, graphicsPipelineCI);
    if (graphicsPipelineResult.result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create graphics pipeline !");
    }
    gData.staticModelPipeline.pipeline = graphicsPipelineResult.value;

    VulkanEngine::gData.device.destroyShaderModule(vertexModule);
    VulkanEngine::gData.device.destroyShaderModule(fragmentModule);

    return true;
}

void VulkanEngine::staticModelPipelineCleanup()
{
    spdlog::info("Static model pipeline cleanup.");
    VulkanEngine::gData.device.destroyDescriptorSetLayout(gData.staticModelPipeline.imageDescriptorLayout);
    VulkanEngine::gData.device.destroyPipeline(gData.staticModelPipeline.pipeline);
    VulkanEngine::gData.device.destroyPipelineLayout(gData.staticModelPipeline.layout);
}