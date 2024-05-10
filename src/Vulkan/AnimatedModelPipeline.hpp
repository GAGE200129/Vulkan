#pragma once

#include "VulkanBuffer.hpp"

class AnimatedModelPipeline
{
public:
    void init();
    void cleanup();

public:
    vk::PipelineLayout mLayout;
    vk::Pipeline mPipeline;
    vk::DescriptorSetLayout mImageDescriptorLayout, mBoneTransformDescriptorLayout;
};