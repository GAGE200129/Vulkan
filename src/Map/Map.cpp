#include "pch.hpp"
#include "Map.hpp"

#include <stb/stb_image.h>

#include "Physics/BulletEngine.hpp"

struct MapData Map::gData = {};

void Map::init()
{
    gData.boxes.reserve(EngineConstants::MAP_MAX_BOXES);
}

void Map::render()
{
    vk::CommandBuffer &cmdBuffer = VulkanEngine::gData.commandBuffer;
    vk::Pipeline &pipeline = VulkanEngine::gData.mapPipeline.pipeline;
    vk::PipelineLayout &pipelineLayout = VulkanEngine::gData.mapPipeline.layout;

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, {VulkanEngine::gData.globalDescriptorSet}, {});
    for (Box &box : gData.boxes)
    {
        for (int i = 0; i < 6; i++)
        {
            Face &f = box.faces[i];
            if (!f.avalidable)
                continue;

            cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                         pipelineLayout, 1,
                                         {f.texture.descriptorSet}, {});
            cmdBuffer.bindVertexBuffers(0, f.vertexBuffer.buffer, {0});
            cmdBuffer.draw(f.vertexCount, 1, 0, 0);
        }
    }
}

void Map::cleanup()
{
    for (Box &box : gData.boxes)
    {
        for (int i = 0; i < 6; i++)
        {
            Face &f = box.faces[i];
            if (f.avalidable)
            {
                VulkanEngine::bufferCleanup(f.vertexBuffer);
            }
        }

        BulletEngine::getWorld()->removeCollisionObject(box.physicsCollider);
        if (box.physicsCollider->getCollisionShape())
            delete box.physicsCollider->getCollisionShape();

        delete box.physicsCollider;
    }

    for (auto &[filePath, texture] : gData.textures)
    {
        VulkanEngine::textureCleanup(texture);
    }
    gData.boxes.clear();
}

VulkanTexture Map::getTexture(const std::string &filePath)
{
    if (gData.textures.find(filePath) != gData.textures.end())
    {
        return gData.textures.at(filePath);
    }
    else
    {
        VulkanTexture texture;
        VulkanEngine::textureLoadFromFile(filePath, VulkanEngine::gData.mapPipeline.diffuseDescriptorLayout, texture);
        gData.textures.insert({filePath, texture});
        return texture;
    }
}
