#pragma once

#include <Vulkan/VulkanEngine.hpp>

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct StaticModelData
{
    struct MeshEntry
    {
        unsigned int numIndices;
        unsigned int baseVertex;
        unsigned int baseIndex;
        unsigned int materialIndex;
    };

    struct Material
    {
        VulkanTexture mDiffuse;
    };
    std::vector<MeshEntry> mMeshes;
    std::vector<Material> mMaterials;
    VulkanBuffer mPositionBuffer, mNormalBuffer, mUvBuffer, mIndexBuffer;
    std::vector<glm::vec3> mPositions;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec2> mUvs;
    std::vector<unsigned int> mIndices;
};

namespace StaticModelLoader
{
    StaticModelData *get(const std::string& filePath);
    StaticModelData *initCache(const std::string &filePath);
    void clearCache();
    bool populateMeshData(StaticModelData* modelData, const aiScene* scene);
    bool populateTextures(StaticModelData* modelData, const aiScene* scene);
}