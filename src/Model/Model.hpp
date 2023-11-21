#pragma once

#include <string>

#include "Vulkan/VulkanBuffer.hpp"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class Model
{
public:
  Model(VulkanEngine& engine): mPositionBuffer(engine), mNormalBuffer(engine), mUvBuffer(engine), mIndexBuffer(engine)
  {}

  void init(const std::string& fileName);
  void cleanup()
  {
    mPositionBuffer.cleanup();
    mNormalBuffer.cleanup();
    mUvBuffer.cleanup();
    mIndexBuffer.cleanup();
  }

public:
struct MeshEntry
  {
    unsigned int numIndices;
    unsigned int baseVertex;
    unsigned int baseIndex;
    unsigned int materialIndex;
  };
  std::vector<MeshEntry> mMeshes;
  VulkanBuffer mPositionBuffer, mNormalBuffer, mUvBuffer, mIndexBuffer;
private:
  std::vector<glm::vec3> mPositions;
  std::vector<glm::vec3> mNormals;
  std::vector<glm::vec2> mUvs;
  std::vector<unsigned int> mIndices;

};