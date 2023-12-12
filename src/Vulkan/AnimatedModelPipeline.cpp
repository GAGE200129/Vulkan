#include "pch.hpp"
#include "AnimatedModelPipeline.hpp"

#include "VulkanEngine.hpp"

void AnimatedModelPipeline::init()
{
  // Descriptor layout
  vk::DescriptorSetLayoutBinding layoutBinding;
  vk::DescriptorSetLayoutCreateInfo layoutCI;

  //Create material
  layoutBinding.setBinding(0)
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(1)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment);

  layoutCI.setBindings(layoutBinding);
  mImageDescriptorLayout = VulkanEngine::mDevice.createDescriptorSetLayout(layoutCI);

  //Create bone transforms
  layoutBinding.setBinding(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(1)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex);

  layoutCI.setBindings(layoutBinding);
  mBoneTransformDescriptorLayout = VulkanEngine::mDevice.createDescriptorSetLayout(layoutCI);

  // Pipeline
  auto vertexCode = VulkanEngine::readfile("res/shaders/animated_model.vert.spv");
  auto fragmentCode = VulkanEngine::readfile("res/shaders/animated_model.frag.spv");

  vk::ShaderModule vertexModule = VulkanEngine::initShaderModule(vertexCode);
  vk::ShaderModule fragmentModule = VulkanEngine::initShaderModule(fragmentCode);
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
  std::array<vk::VertexInputBindingDescription, 5> vertexInputBindingDescriptions;
  vertexInputBindingDescriptions[0].setBinding(0).setStride(sizeof(glm::vec3)).setInputRate(vk::VertexInputRate::eVertex);
  vertexInputBindingDescriptions[1].setBinding(1).setStride(sizeof(glm::vec3)).setInputRate(vk::VertexInputRate::eVertex);
  vertexInputBindingDescriptions[2].setBinding(2).setStride(sizeof(glm::vec2)).setInputRate(vk::VertexInputRate::eVertex);
  vertexInputBindingDescriptions[3].setBinding(3).setStride(sizeof(glm::ivec4)).setInputRate(vk::VertexInputRate::eVertex);
  vertexInputBindingDescriptions[4].setBinding(4).setStride(sizeof(glm::vec4)).setInputRate(vk::VertexInputRate::eVertex);

  std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttributeDescription;
  vertexInputAttributeDescription[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat);
  vertexInputAttributeDescription[1].setBinding(1).setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat);
  vertexInputAttributeDescription[2].setBinding(2).setLocation(2).setFormat(vk::Format::eR32G32Sfloat);
  vertexInputAttributeDescription[3].setBinding(3).setLocation(3).setFormat(vk::Format::eR32G32B32A32Uint);
  vertexInputAttributeDescription[4].setBinding(4).setLocation(4).setFormat(vk::Format::eR32G32B32A32Sfloat);

  vertexInputCI.setVertexBindingDescriptions(vertexInputBindingDescriptions)
      .setVertexAttributeDescriptions(vertexInputAttributeDescription);
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCI;
  inputAssemblyCI.setTopology(vk::PrimitiveTopology::eTriangleList)
      .setPrimitiveRestartEnable(false);
  vk::Viewport viewport;
  viewport.setX(0)
      .setY(0)
      .setWidth(VulkanEngine::mSwapExtent.width)
      .setHeight(VulkanEngine::mSwapExtent.height)
      .setMinDepth(0.0f)
      .setMaxDepth(1.0f);
  vk::Rect2D scissor;
  scissor.setOffset({0, 0})
      .setExtent(VulkanEngine::mSwapExtent);
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

  vk::PushConstantRange ps;
  ps.setOffset(0).setSize(sizeof(glm::mat4x4)).setStageFlags(vk::ShaderStageFlagBits::eVertex);
  vk::PipelineLayoutCreateInfo pipelineLayoutCI;

  auto layouts = std::array{VulkanEngine::mGlobalDescriptorLayout, mImageDescriptorLayout, mBoneTransformDescriptorLayout};
  pipelineLayoutCI.setSetLayouts(layouts)
      .setPushConstantRanges(ps);

  mLayout = VulkanEngine::mDevice.createPipelineLayout(pipelineLayoutCI);
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
      .setLayout(mLayout)
      .setRenderPass(VulkanEngine::mRenderPass)
      .setSubpass(0)
      .setBasePipelineHandle(nullptr)
      .setBasePipelineIndex(-1);

  auto graphicsPipelineResult = VulkanEngine::mDevice.createGraphicsPipeline(nullptr, graphicsPipelineCI);
  if (graphicsPipelineResult.result != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create graphics pipeline !");
  }
  mPipeline = graphicsPipelineResult.value;

  VulkanEngine::mDevice.destroyShaderModule(vertexModule);
  VulkanEngine::mDevice.destroyShaderModule(fragmentModule);

}


void AnimatedModelPipeline::cleanup()
{
  VulkanEngine::mDevice.destroyDescriptorSetLayout(mImageDescriptorLayout);
  VulkanEngine::mDevice.destroyDescriptorSetLayout(mBoneTransformDescriptorLayout);
  VulkanEngine::mDevice.destroyPipeline(mPipeline);
  VulkanEngine::mDevice.destroyPipelineLayout(mLayout);
}
