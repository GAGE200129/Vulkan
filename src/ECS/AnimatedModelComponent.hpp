#pragma once

#include "Components.hpp"
#include "TransformComponent.hpp"

#include "Vulkan/VulkanTexture.hpp"

class AnimatedModelComponent : public Component
{
public:
  AnimatedModelComponent(const std::string &filePath) : mFilePath(filePath) {}
  void init() override
  {
    if (sCache.find(mFilePath) != sCache.end())
    {
      mMeshData = sCache.at(mFilePath).get();
    }
    else
    {
      mMeshData = initCache(mFilePath);
    }

    mTransformComponent = mGameObject->getRequiredComponent<TransformComponent>();
  }

  void render() override;

private:
  struct MeshData
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

    struct BoneData
    {
      glm::uvec4 ids;
      glm::vec4 weights;
    };
    std::vector<MeshEntry> mMeshes;
    std::vector<Material> mMaterials;
    VulkanBuffer mPositionBuffer, mNormalBuffer, mUvBuffer, mBoneIDBuffer, mBoneWeightBuffer, mIndexBuffer;
    std::vector<glm::vec3> mPositions;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec2> mUvs;
    std::vector<glm::uvec4> mBoneIDs;
    std::vector<glm::vec4> mBoneWeights;
    std::vector<unsigned int> mIndices;

    std::vector<unsigned int> mMeshBaseVertex;
    std::map<std::string, unsigned int> mBoneNameToIndex;
  };
  MeshData *mMeshData;
  TransformComponent *mTransformComponent;
  std::string mFilePath;

  // static field
public:
  static MeshData *initCache(const std::string &filePath);
  static void clearCache();

private:
  static void parseBone(const std::unique_ptr<MeshData>& meshData, unsigned int meshID, aiBone* pBone);
private:
  static std::map<std::string, std::unique_ptr<MeshData>> sCache;
};