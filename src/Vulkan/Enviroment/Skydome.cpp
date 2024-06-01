#include "pch.hpp"
#include "../VulkanEngine.hpp"

const glm::vec3 gSphereCoordToCartesian(float radius, float pitch, float yaw);
void gGenerateVertices(std::vector<VulkanSkydome::Vertex> &vertices, const int numRows, const int numCols, const float radius);

static VulkanSkydome gSkydome = {};

void VulkanEngine::skydomeInit()
{
    gGenerateVertices(gSkydome.vertices, 20, 20, 1.0f);

    VkDeviceSize size = gSkydome.vertices.size() * sizeof(VulkanSkydome::Vertex);
    VulkanEngine::bufferInitAndTransferToLocalDevice(gSkydome.vertices.data(), size, vk::BufferUsageFlagBits::eVertexBuffer, gSkydome.buffer);

}
void VulkanEngine::skydomeRender()
{
    vk::CommandBuffer &cmdBuffer = VulkanEngine::gData.commandBuffer;
    vk::Pipeline &pipeline = VulkanEngine::gData.skydomePipeline.pipeline;
    vk::PipelineLayout &pipelineLayout = VulkanEngine::gData.skydomePipeline.layout;

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, {VulkanEngine::gData.globalDescriptorSet}, {});
    cmdBuffer.bindVertexBuffers(0, gSkydome.buffer.buffer, {0});
    cmdBuffer.draw(gSkydome.vertices.size(), 1, 0, 0);
}

void VulkanEngine::skydomeCleanup()
{
    VulkanEngine::bufferCleanup(gSkydome.buffer);
}

void gGenerateVertices(std::vector<VulkanSkydome::Vertex> &vertices, const int numRows, const int numCols, const float radius)
{
    const int topStripNumVertices = numCols * 3;
    const int stripNumVertices = numCols * 6;
    const int vertexCount = topStripNumVertices + (numRows - 1) * stripNumVertices;
    vertices.resize(vertexCount);

    const float deltaPitch = 90.0f / (float)numRows;
    const float deltaYaw = 360.0f / (float)numCols;

    glm::vec3 apex = {0.0f, radius, 0.0f};
    
    int vertexIndex = 0;
    // Generate top
    for (float yaw = 0.0f; yaw < 360.0f; yaw += deltaYaw)
    {
        const float pitch = 90.0f;
        const glm::vec3 v0 = apex;
        const glm::vec3 v1 = gSphereCoordToCartesian(radius, pitch - deltaPitch, yaw + deltaYaw);
        const glm::vec3 v2 = gSphereCoordToCartesian(radius, pitch - deltaPitch, yaw);

        vertices[vertexIndex++].position = v0;
        vertices[vertexIndex++].position = v1;
        vertices[vertexIndex++].position = v2;
    }

    // Generate rest
    for (float pitch = 90.0f - deltaPitch; pitch > 0.0f; pitch -= deltaPitch)
    {
        for (float yaw = 0.0f; yaw < 360; yaw += deltaYaw)
        {
            const glm::vec3 v0 = gSphereCoordToCartesian(radius, pitch, yaw);
            const glm::vec3 v1 = gSphereCoordToCartesian(radius, pitch, yaw + deltaYaw);
            const glm::vec3 v2 = gSphereCoordToCartesian(radius, pitch - deltaPitch, yaw);
            const glm::vec3 v3 = gSphereCoordToCartesian(radius, pitch - deltaPitch, yaw + deltaYaw);

            vertices[vertexIndex++].position = v0;
            vertices[vertexIndex++].position = v1;
            vertices[vertexIndex++].position = v2;

            vertices[vertexIndex++].position = v1;
            vertices[vertexIndex++].position = v3;
            vertices[vertexIndex++].position = v2;
        }
    }
}

const glm::vec3 gSphereCoordToCartesian(float radius, float pitch, float yaw)
{
    float x, y, z;
    x = radius * glm::sin(glm::radians(pitch)) * glm::cos(glm::radians(yaw));
    y = radius * glm::cos(glm::radians(pitch));
    z = radius * glm::sin(glm::radians(pitch)) * glm::sin(glm::radians(yaw));

    return {x, y, z};
}
