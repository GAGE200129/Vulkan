#pragma once

#include "Components.hpp"
#include "GameObject.hpp"
#include "TransformComponent.hpp"

#include <vector>
#include <map>
#include <Vulkan/VulkanBuffer.hpp>
#include <Vulkan/VulkanTexture.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>




class ModelComponent : public Component
{
public:
  ModelComponent(const std::string& filePath) : mFilePath(filePath) {}
  void init() override 
  {
    if(sCache.find(mFilePath) != sCache.end())
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
    std::vector<MeshEntry> mMeshes;
    std::vector<VulkanTexture> mTextures;
    VulkanBuffer mPositionBuffer, mNormalBuffer, mUvBuffer, mIndexBuffer;
    std::vector<glm::vec3> mPositions;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec2> mUvs;
    std::vector<unsigned int> mIndices;
  };
  MeshData* mMeshData;
  TransformComponent* mTransformComponent;
  std::string mFilePath;

//static field
public:
  static MeshData* initCache(const std::string& filePath);
  static void clearCache();
private:
  static std::map<std::string, std::unique_ptr<MeshData>> sCache;
};