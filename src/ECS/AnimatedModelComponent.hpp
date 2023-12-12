#pragma once

#include "Components.hpp"
#include "TransformComponent.hpp"

#include "Vulkan/VulkanTexture.hpp"

#include <btBulletDynamicsCommon.h>

class AnimatedModelComponent : public Component
{
public:
  AnimatedModelComponent(const std::string &filePath) : mFilePath(filePath) {}
  void init() override;
  void update(float delta) override;
  void render() override;
  void shutdown() noexcept override;

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

    Assimp::Importer mImporter;
    const aiScene* mPScene;
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
    std::vector<glm::mat4> mBoneIndexToOffset;
  };

  MeshData *mMeshData;
  TransformComponent *mTransformComponent;
  std::string mFilePath;
  std::array<glm::mat4, 100> mBoneTransforms;
  VulkanBuffer mBoneTransformBuffer;
  void* mBoneTransformBufferMapped;
  vk::DescriptorSet mBoneTransformDescriptorSet;

  // static field
public:
  static MeshData *initCache(const std::string &filePath);
  static void clearCache();

private:
  static void parseBone(const std::unique_ptr<MeshData>& meshData, unsigned int meshID, aiBone* pBone);
private:
  static std::map<std::string, std::unique_ptr<MeshData>> sCache;
};