#include "ModelComponent.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Vulkan/VulkanEngine.hpp>

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "Vulkan/VulkanTexture.hpp"

std::map<std::string, std::unique_ptr<ModelComponent::MeshData>> ModelComponent::sCache;

ModelComponent::MeshData *ModelComponent::initCache(const std::string &filePath)
{
  Assimp::Importer importer;

  const aiScene *pScene = importer.ReadFile(filePath,
                                            aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

  if (pScene == nullptr)
  {
    throw std::runtime_error(importer.GetErrorString());
  }

  auto meshData = std::make_unique<MeshData>();
  // Count and reserve vertex data for all meshes in scene
  unsigned int numVertices = 0, numIndices = 0;
  meshData->mMeshes.resize(pScene->mNumMeshes);
  for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
  {
    meshData->mMeshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
    meshData->mMeshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
    meshData->mMeshes[i].baseVertex = numVertices;
    meshData->mMeshes[i].baseIndex = numIndices;

    numVertices += pScene->mMeshes[i]->mNumVertices;
    numIndices += meshData->mMeshes[i].numIndices;
  }
  meshData->mPositions.reserve(numVertices);
  meshData->mNormals.reserve(numVertices);
  meshData->mUvs.reserve(numVertices);
  meshData->mIndices.reserve(numIndices);

  // Populate mesh datas
  for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
  {
    const aiMesh *mesh = pScene->mMeshes[i];

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      const aiVector3D &pos = mesh->mVertices[i];
      const aiVector3D &normal = mesh->mNormals[i];
      const aiVector3D &uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0, 0, 0);

      meshData->mPositions.push_back({pos.x, pos.y, pos.z});
      meshData->mNormals.push_back({normal.x, normal.y, normal.z});
      meshData->mUvs.push_back({uv.x, uv.y});
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      const aiFace &face = mesh->mFaces[i];
      if (face.mNumIndices != 3)
        throw std::runtime_error("Wrong face type !");
      meshData->mIndices.push_back(face.mIndices[0]);
      meshData->mIndices.push_back(face.mIndices[1]);
      meshData->mIndices.push_back(face.mIndices[2]);
    }
  }

  //Init texture

  for(int i = 0; i < pScene->mNumMaterials; i++)
  {
    const auto* aiMaterial = pScene->mMaterials[i];
    aiString textureFile;
    //aiMaterial->Get(AI_MATKEY_TEXTURE

  

  }

  // Push mesh data to vulkan buffer
  meshData->mPositionBuffer.initAndTransferToLocalDevice(meshData->mPositions.data(),
                                                         meshData->mPositions.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mNormalBuffer.initAndTransferToLocalDevice(meshData->mNormals.data(),
                                                       meshData->mNormals.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mUvBuffer.initAndTransferToLocalDevice(meshData->mUvs.data(),
                                                   meshData->mUvs.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mIndexBuffer.initAndTransferToLocalDevice(meshData->mIndices.data(),
                                                      meshData->mIndices.size() * sizeof(unsigned int), vk::BufferUsageFlagBits::eIndexBuffer);
  auto ptr = meshData.get();
  sCache[filePath] = std::move(meshData);

  std::cout << "Caching model: " << filePath << "\n";
  return ptr;
}

void ModelComponent::render()
{
  static VulkanTexture texture;
  static bool first = true;
  if(first)
  {
    texture.loadFromFile("res/models/adamHead/Assets/Models/PBR/Adam/Textures/Adam_Head_a.jpg");
    first = false;
  }
  VulkanEngine::mCommandBuffer.bindVertexBuffers(0, mMeshData->mPositionBuffer.getBuffer(), {0});
  VulkanEngine::mCommandBuffer.bindVertexBuffers(1, mMeshData->mNormalBuffer.getBuffer(), {0});
  VulkanEngine::mCommandBuffer.bindVertexBuffers(2, mMeshData->mUvBuffer.getBuffer(), {0});
  VulkanEngine::mCommandBuffer.bindIndexBuffer(mMeshData->mIndexBuffer.getBuffer(), 0, vk::IndexType::eUint32);
  glm::mat4x4 modelMat = glm::translate(glm::mat4(1.0f), mTransformComponent->position);
  VulkanEngine::mCommandBuffer.pushConstants(VulkanEngine::mPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(modelMat), &modelMat);
  for (const auto &mesh : mMeshData->mMeshes)
  {
    VulkanEngine::mCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, VulkanEngine::mPipelineLayout, 
        0, {VulkanEngine::mGlobalDescriptorSet, texture.mDescriptorSet}, {});
    VulkanEngine::mCommandBuffer.drawIndexed(mesh.numIndices, 1, mesh.baseIndex, mesh.baseVertex, 0);
  }
}

void ModelComponent::clearCache()
{
  for (auto &[path, mesh] : sCache)
  {
    mesh->mPositionBuffer.cleanup();
    mesh->mNormalBuffer.cleanup();
    mesh->mUvBuffer.cleanup();
    mesh->mIndexBuffer.cleanup();
  }
  sCache.clear();
}
