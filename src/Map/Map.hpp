#pragma once

#include "EngineConstants.hpp"
#include "Vulkan/VulkanEngine.hpp"
#include "EngineConstants.hpp"


struct MapVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Face
{
    char                    texturePath[EngineConstants::PATH_LENGTH];
    int                     textureWidth;
    int                     textureHeight;
    VulkanTexture           texture;
    float                   scaleX, scaleY;
    VulkanBuffer            vertexBuffer;
    int                     vertexCount;
    bool                    avalidable;
};

struct Box
{
    glm::vec3 center;
    glm::vec3 halfSize;
    btCollisionObject *physicsCollider;
    //Order top, bottom, front, back, left, right
    Face faces[6];
};

struct MapData
{
    std::vector<Box>   boxes;
    std::unordered_map<std::string, VulkanTexture> textures;
};

namespace Map
{
    void init();

    void render();
    void cleanup();

    VulkanTexture getTexture(const std::string& filePath);

    void boxAdd(Box box);
    void boxUpdate(Box* box);

    extern MapData gData;
};