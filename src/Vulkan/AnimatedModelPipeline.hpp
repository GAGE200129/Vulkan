#pragma once

#include "VulkanBuffer.hpp"

class AnimatedModelPipeline
{
public:
  void init();
  void setup();
  void cleanup();
public:
  vk::PipelineLayout mLayout;
  vk::Pipeline mPipeline;
  vk::DescriptorSetLayout mGlobalDescriptorLayout, mImageDescriptorLayout;
  vk::DescriptorSet mGlobalDescriptorSet;
  VulkanBuffer mUniformBuffer;
  void *mUniformBufferMap;
};