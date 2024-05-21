#include "pch.hpp"
#include "StaticModelComponent.hpp"

#include "Vulkan/VulkanEngine.hpp"

#include <stb/stb_image.h>

void StaticModelComponent::render()
{
    vk::CommandBuffer &cmdBuffer = VulkanEngine::gData.commandBuffer;
    vk::Pipeline &pipeline = VulkanEngine::gData.staticModelPipeline.pipeline;
    vk::PipelineLayout &pipelineLayout = VulkanEngine::gData.staticModelPipeline.layout;

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, {VulkanEngine::gData.globalDescriptorSet}, {});
    cmdBuffer.bindVertexBuffers(0, mMeshData->mPositionBuffer.buffer, {0});
    cmdBuffer.bindVertexBuffers(1, mMeshData->mNormalBuffer.buffer, {0});
    cmdBuffer.bindVertexBuffers(2, mMeshData->mUvBuffer.buffer, {0});
    cmdBuffer.bindIndexBuffer(mMeshData->mIndexBuffer.buffer, 0, vk::IndexType::eUint32);
    const glm::mat4x4 modelMat = mGameObject->buildTransform();
    cmdBuffer.pushConstants(pipelineLayout,
                            vk::ShaderStageFlagBits::eVertex, 0, sizeof(modelMat), &modelMat);
    for (const auto &mesh : mMeshData->mMeshes)
    {
        auto &material = mMeshData->mMaterials[mesh.materialIndex];

        cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                                     1, {material.mDiffuse.descriptorSet}, {});
        cmdBuffer.drawIndexed(mesh.numIndices, 1, mesh.baseIndex, mesh.baseVertex, 0);
    }
}

