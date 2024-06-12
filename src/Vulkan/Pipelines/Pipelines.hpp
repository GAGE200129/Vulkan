#pragma once

struct VulkanStaticModelPipeline
{
    vk::PipelineLayout layout;
    vk::Pipeline pipeline;
    vk::DescriptorSetLayout imageDescriptorLayout;
};

struct VulkanMapPipeline
{
    vk::PipelineLayout layout;
    vk::Pipeline pipeline;
    vk::DescriptorSetLayout diffuseDescriptorLayout;
};

struct VulkanSkydomePipeline
{
    vk::PipelineLayout layout;
    vk::Pipeline pipeline;
    vk::DescriptorSetLayout descriptorLayout;
}; 

struct VulkanRaymarchPipeline
{
    vk::PipelineLayout layout;
    vk::Pipeline pipeline;
    vk::DescriptorSetLayout descriptorLayout;
}; 
