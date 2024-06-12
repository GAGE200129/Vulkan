#include "pch.hpp"
#include "../VulkanEngine.hpp"

#include "EngineConstants.hpp"

static VulkanBuffer gUniformBuffer, gVertexBuffer;
static void *gUniformBufferMap;
static vk::DescriptorSet gDescriptorSet;
static float gTime = 0.0f;

struct RaymarchUBO
{
    int32_t screenWidth, screenHeight;
    float cameraNear, cameraFar;
    glm::vec3 cameraPosition;
    float time;
    glm::mat4 cameraRotation;
};

bool VulkanEngine::raymarchInit()
{
    vk::DeviceSize size = sizeof(RaymarchUBO);
    VulkanEngine::bufferInit(size, vk::BufferUsageFlagBits::eUniformBuffer,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                             gUniformBuffer);
    gUniformBufferMap = VulkanEngine::bufferGetMapped(gUniformBuffer, 0, size);

    // Descriptor
    vk::DescriptorSetAllocateInfo dsAI;
    dsAI.setDescriptorPool(gData.descriptorPool)
        .setSetLayouts(gData.globalDescriptorLayout);

    auto [dsResult, ds] = gData.device.allocateDescriptorSets(dsAI);
    if (dsResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create descriptor set: {}", vk::to_string(dsResult));
        return false;
    }
    gDescriptorSet = ds[0];

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setBuffer(gUniformBuffer.buffer)
        .setOffset(0)
        .setRange(sizeof(RaymarchUBO));
    vk::WriteDescriptorSet writeDescriptor;
    writeDescriptor.setDstSet(gDescriptorSet)
        .setDstBinding(0)
        .setDstArrayElement(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setBufferInfo(bufferInfo);
    gData.device.updateDescriptorSets(writeDescriptor, {});

    glm::vec3 positions[] =
        {
            {-1.0f, -1.0f, 0.0f},
            {-1.0f, 3.0f, 0.0f},
            {3.0f, -1.0f, 0.0f}};

    bufferInitAndTransferToLocalDevice(positions,
                                       sizeof(positions),
                                       vk::BufferUsageFlagBits::eVertexBuffer,
                                       gVertexBuffer);

    return true;
}

void VulkanEngine::raymarchUpdate()
{
    gTime += EngineConstants::TICK_TIME;
    if (gTime > 60.0f)
        gTime = 0.0f;
}

void VulkanEngine::raymarchRender(const Camera &camera)
{
    vk::CommandBuffer &cmdBuffer = VulkanEngine::gData.commandBuffer;
    vk::Pipeline &pipeline = VulkanEngine::gData.raymarchPipeline.pipeline;
    vk::PipelineLayout &pipelineLayout = VulkanEngine::gData.raymarchPipeline.layout;

    // Update ubo
    int width, height;
    glfwGetFramebufferSize(gData.window, &width, &height);
    static RaymarchUBO ubo;
    ubo.screenWidth = width;
    ubo.screenHeight = height;
    ubo.cameraPosition = camera.position;
    ubo.cameraRotation = camera.getViewInvert();
    ubo.cameraNear = camera.nearPlane;
    ubo.cameraFar = camera.farPlane;
    ubo.time = gTime;
    std::memcpy(gUniformBufferMap, &ubo, sizeof(ubo));

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                                 0, {gDescriptorSet}, {});
    cmdBuffer.bindVertexBuffers(0, gVertexBuffer.buffer, {0});
    cmdBuffer.draw(3, 1, 0, 0);
}

void VulkanEngine::raymarchCleanup()
{
    bufferCleanup(gUniformBuffer);
    bufferCleanup(gVertexBuffer);
}