#pragma once

#include <Vulkan/VulkanEngine.hpp>

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct StaticMeshData
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

class StaticModelLoader
{
public:
    static StaticMeshData *get(const std::string& filePath);
    static StaticMeshData *initCache(const std::string &filePath);
    static void clearCache();
private:
    static bool populateMeshData(StaticMeshData* meshData, const aiScene* scene);
    static bool populateTextures(StaticMeshData* meshData, const aiScene* scene);
private:
    static std::map<std::string, std::unique_ptr<StaticMeshData>> sCache;
};