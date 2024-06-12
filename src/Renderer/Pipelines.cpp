#include "pch.hpp"
#include "Renderer.hpp"

bool Renderer::pipelinesInit()
{
    if(!backgroundPipelineInit())
        return false;


    return true;
}


bool Renderer::backgroundPipelineInit()
{
    vk::PipelineLayoutCreateInfo computeLayout = {};
	computeLayout.pSetLayouts = &gData.drawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

    auto result = gData.device.createPipelineLayout(computeLayout);
    if(result.result != vk::Result::eSuccess)
    {
        gLogger->critical("Failed to create background pipeline layout: {}", vk::to_string(result.result));
        return false;
    }
    gData.gradientPipelineLayout = result.value;

    auto computeDrawShader = loadShaderModule("res/shaders/gradient.comp.spv");
	if(!computeDrawShader.has_value())
    {
        gLogger->critical("Failed to load shader module !");
        return false;
    }

    vk::PipelineShaderStageCreateInfo stageinfo = {};
	stageinfo.stage = vk::ShaderStageFlagBits::eCompute;
	stageinfo.module = computeDrawShader.value();
	stageinfo.pName = "main";

    vk::ComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.layout = gData.gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

    auto pipelineResult = gData.device.createComputePipeline(nullptr, computePipelineCreateInfo);
    if(pipelineResult.result != vk::Result::eSuccess)
    {
        gLogger->critical("Failed to create pipeline: {}", vk::to_string(pipelineResult.result));
        return false;
    }

    gData.gradientPipeline = pipelineResult.value;

    gData.device.destroyShaderModule(computeDrawShader.value());

    gData.mainDeletionQueue.push_back([]()
    {
        gData.device.destroyPipeline(gData.gradientPipeline);
        gData.device.destroyPipelineLayout(gData.gradientPipelineLayout);
    });


    return true;
}