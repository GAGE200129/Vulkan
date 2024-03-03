#include "pch.hpp"
#include "AnimatedModelComponent.hpp"

#include "Vulkan/VulkanEngine.hpp"
#include "Vulkan/VulkanTexture.hpp"

#include <stb/stb_image.h>
#include <GL/gl.h>

std::map<std::string, std::unique_ptr<AnimatedModelComponent::MeshData>> AnimatedModelComponent::sCache;

AnimatedModelComponent::MeshData *AnimatedModelComponent::initCache(const std::string &filePath)
{
  std::cout << "Caching model: " << filePath << "\n";
  auto meshData = std::make_unique<MeshData>();

  const aiScene *pScene = meshData->mImporter.ReadFile(filePath,
                                                       aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
  if (pScene == nullptr)
  {
    throw std::runtime_error(meshData->mImporter.GetErrorString());
  }
  meshData->mPScene = pScene;
  // meshData->mGlobalInverse = glm::inverse(Utils::aiToGlmMatrix4x4(pScene->mRootNode->mTransformation));

  // Count and reserve vertex data for all meshes in scene
  unsigned int numVertices = 0, numIndices = 0, numBones = 0;
  meshData->mMeshes.resize(pScene->mNumMeshes);

  for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
  {
    meshData->mMeshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
    meshData->mMeshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
    meshData->mMeshes[i].baseVertex = numVertices;
    meshData->mMeshes[i].baseIndex = numIndices;

    meshData->mMeshBaseVertex.push_back(numVertices);

    numVertices += pScene->mMeshes[i]->mNumVertices;
    numIndices += meshData->mMeshes[i].numIndices;
    numBones += pScene->mMeshes[i]->mNumBones;

    std::cout << "Mesh: " << pScene->mMeshes[i]->mName.C_Str()
              << " vertices: " << numVertices
              << " indices: " << numIndices
              << " bones: " << numBones << "\n";
  }

  meshData->mBoneIDs.resize(numVertices);
  meshData->mBoneWeights.resize(numVertices);
  meshData->mPositions.reserve(numVertices);
  meshData->mNormals.reserve(numVertices);
  meshData->mUvs.reserve(numVertices);
  meshData->mIndices.reserve(numIndices);

  // Populate mesh datas
  for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
  {
    const aiMesh *mesh = pScene->mMeshes[i];
    // Parse bone
    for (unsigned int j = 0; j < mesh->mNumBones; j++)
    {
      parseBone(meshData, i, mesh->mBones[j]);
    }
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

  // Init texture

  for (int i = 0; i < pScene->mNumMaterials; i++)
  {
    const auto *aiMaterial = pScene->mMaterials[i];
    // Check for assimp's default material
    if (aiMaterial->GetName().length == 0)
      continue;

    aiString textureFile;
    aiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureFile);
    const aiTexture *texture = pScene->GetEmbeddedTexture(textureFile.C_Str());
    if (!texture)
      throw std::runtime_error("Model component only accepts embedded textures !");

    int width = texture->mWidth, height = texture->mHeight;
    stbi_uc *imageData = (stbi_uc *)texture->pcData;
    bool needDecoder = false;

    // Raw texture data, need a decoder
    if (texture->mHeight == 0)
    {
      needDecoder = true;
      int bpp;
      imageData = stbi_load_from_memory((stbi_uc *)texture->pcData, texture->mWidth, &width, &height, &bpp, STBI_rgb_alpha);
      if (!imageData)
        throw std::runtime_error("Something wrong with encoded embedded texture !");
    }
    meshData->mMaterials.emplace_back();
    auto &diffuseTexture = meshData->mMaterials.back().mDiffuse;
    size_t imageSize = width * height * 4;
    // Create a staging buffer and copy to it
    VulkanBuffer staging;
    staging.init(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    staging.copy(imageData, imageSize);

    // Create image handle
    diffuseTexture.init(width, height,
                        vk::Format::eR8G8B8A8Srgb,
                        vk::ImageTiling::eOptimal,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                        vk::MemoryPropertyFlagBits::eDeviceLocal, VulkanEngine::mAnimatedModelPipeline.mImageDescriptorLayout);

    // Upload image to GPU
    diffuseTexture.transitionLayout(vk::Format::eR8G8B8Srgb,
                                    vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    diffuseTexture.copyBufferToImage(staging, width, height);
    diffuseTexture.transitionLayout(vk::Format::eR8G8B8Srgb,
                                    vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    staging.cleanup();

    if (needDecoder)
      stbi_image_free(imageData);
  }

  // Push mesh data to vulkan buffer
  meshData->mPositionBuffer.initAndTransferToLocalDevice(meshData->mPositions.data(),
                                                         meshData->mPositions.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mNormalBuffer.initAndTransferToLocalDevice(meshData->mNormals.data(),
                                                       meshData->mNormals.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mUvBuffer.initAndTransferToLocalDevice(meshData->mUvs.data(),
                                                   meshData->mUvs.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mBoneIDBuffer.initAndTransferToLocalDevice(meshData->mBoneIDs.data(),
                                                       meshData->mBoneIDs.size() * sizeof(glm::uvec4), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mBoneWeightBuffer.initAndTransferToLocalDevice(meshData->mBoneWeights.data(),
                                                           meshData->mBoneWeights.size() * sizeof(glm::vec4), vk::BufferUsageFlagBits::eVertexBuffer);

  meshData->mIndexBuffer.initAndTransferToLocalDevice(meshData->mIndices.data(),
                                                      meshData->mIndices.size() * sizeof(unsigned int), vk::BufferUsageFlagBits::eIndexBuffer);
  auto ptr = meshData.get();
  sCache[filePath] = std::move(meshData);

  return ptr;
}

void AnimatedModelComponent::parseBone(const std::unique_ptr<MeshData> &meshData, unsigned int meshID, aiBone *pBone)
{
  std::cout << "Bone :" << pBone->mName.C_Str()
            << " | num vertices affected by this bone: " << pBone->mNumWeights << "\n";

  unsigned int boneID = 0;

  auto &boneNameToIndex = meshData->mBoneNameToIndex;
  std::string stdBoneName = std::string(pBone->mName.C_Str());
  if (boneNameToIndex.find(stdBoneName) == boneNameToIndex.end())
  {
    boneID = boneNameToIndex.size();
    boneNameToIndex[stdBoneName] = boneID;
  }
  else
  {
    boneID = boneNameToIndex.at(stdBoneName);
  }

  if (boneID == meshData->mBoneIndexToOffset.size())
  {
    meshData->mBoneIndexToOffset.push_back(Utils::aiToGlmMatrix4x4(pBone->mOffsetMatrix));
  }

  for (unsigned int i = 0; i < pBone->mNumWeights; i++)
  {
    const aiVertexWeight &vw = pBone->mWeights[i];
    unsigned int globalVertexID = meshData->mMeshBaseVertex[meshID] + vw.mVertexId;
    auto &vertexToBoneID = meshData->mBoneIDs[globalVertexID];
    auto &vertexToBoneWeight = meshData->mBoneWeights[globalVertexID];

    for (unsigned int j = 0; j < 4; j++)
    {
      if (vertexToBoneWeight[j] == 0.0f)
      {
        vertexToBoneID[j] = boneID;
        vertexToBoneWeight[j] = vw.mWeight;
        break;
      }
    }
  }
}

void AnimatedModelComponent::update(float delta)
{
  // Update UBO
  std::memcpy(mBoneTransformBufferMapped, mBoneTransforms.data(), sizeof(glm::mat4x4) * mMeshData->mBoneIndexToOffset.size());
}

void AnimatedModelComponent::debugRender()
{
  glm::mat4 modelMatrix = mTransformComponent->build();
  const std::vector<glm::vec3> &positions = mMeshData->mPositions;
  const std::vector<unsigned int> &indices = mMeshData->mIndices;

  glColor3f(1, 1, 1);
  glBegin(GL_TRIANGLES);
  for (const auto &mesh : mMeshData->mMeshes)
  {
    for (size_t i = 0; i < mesh.numIndices; i++)
    {
      unsigned int index = indices.at(mesh.baseIndex + i);
      const glm::vec3 &position = positions.at(mesh.baseVertex + index);
      glm::vec4 positionTranslated = modelMatrix * glm::vec4(position, 1);

      glVertex3f(positionTranslated.x, positionTranslated.y, positionTranslated.z);
    }
  }
  glEnd();
}
void AnimatedModelComponent::render()
{
  vk::CommandBuffer &cmdBuffer = VulkanEngine::mCommandBuffer;
  vk::Pipeline &pipeline = VulkanEngine::mAnimatedModelPipeline.mPipeline;
  vk::PipelineLayout &pipelineLayout = VulkanEngine::mAnimatedModelPipeline.mLayout;
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
  cmdBuffer.bindVertexBuffers(0, mMeshData->mPositionBuffer.getBuffer(), {0});
  cmdBuffer.bindVertexBuffers(1, mMeshData->mNormalBuffer.getBuffer(), {0});
  cmdBuffer.bindVertexBuffers(2, mMeshData->mUvBuffer.getBuffer(), {0});
  cmdBuffer.bindVertexBuffers(3, mMeshData->mBoneIDBuffer.getBuffer(), {0});
  cmdBuffer.bindVertexBuffers(4, mMeshData->mBoneWeightBuffer.getBuffer(), {0});
  cmdBuffer.bindIndexBuffer(mMeshData->mIndexBuffer.getBuffer(), 0, vk::IndexType::eUint32);
  glm::mat4x4 modelMat = mTransformComponent->build();
  cmdBuffer.pushConstants(pipelineLayout,
                          vk::ShaderStageFlagBits::eVertex, 0, sizeof(modelMat), &modelMat);
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                               2, {mBoneTransformDescriptorSet}, {});
  for (const auto &mesh : mMeshData->mMeshes)
  {
    auto &material = mMeshData->mMaterials[mesh.materialIndex];

    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, VulkanEngine::VulkanEngine::mAnimatedModelPipeline.mLayout,
                                 1, {material.mDiffuse.mDescriptorSet}, {});
    cmdBuffer.drawIndexed(mesh.numIndices, 1, mesh.baseIndex, mesh.baseVertex, 0);
  }
}

void AnimatedModelComponent::clearCache()
{
  for (auto &[path, mesh] : sCache)
  {
    mesh->mPositionBuffer.cleanup();
    mesh->mNormalBuffer.cleanup();
    mesh->mUvBuffer.cleanup();
    mesh->mIndexBuffer.cleanup();
    mesh->mBoneIDBuffer.cleanup();
    mesh->mBoneWeightBuffer.cleanup();
    for (auto &material : mesh->mMaterials)
      material.mDiffuse.cleanup();
    mesh->mImporter.FreeScene();
  }
  sCache.clear();
}

void AnimatedModelComponent::init()
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

  // Uniform buffer
  vk::DeviceSize size = sizeof(mBoneTransforms);
  mBoneTransformBuffer.init(size, vk::BufferUsageFlagBits::eUniformBuffer,
                            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  mBoneTransformBufferMapped = mBoneTransformBuffer.getMapped(0, size);

  // Descriptor
  vk::DescriptorSetAllocateInfo dsAI;
  dsAI.setDescriptorPool(VulkanEngine::mDescriptorPool)
      .setSetLayouts(VulkanEngine::mAnimatedModelPipeline.mBoneTransformDescriptorLayout);
  mBoneTransformDescriptorSet = VulkanEngine::mDevice.allocateDescriptorSets(dsAI)[0];
  vk::DescriptorBufferInfo bufferInfo;
  bufferInfo.setBuffer(mBoneTransformBuffer.getBuffer())
      .setOffset(0)
      .setRange(size);
  vk::WriteDescriptorSet writeDescriptor;
  writeDescriptor.setDstSet(mBoneTransformDescriptorSet)
      .setDstBinding(0)
      .setDstArrayElement(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(1)
      .setBufferInfo(bufferInfo);
  VulkanEngine::mDevice.updateDescriptorSets(writeDescriptor, {});
}
void AnimatedModelComponent::shutdown() noexcept
{
  mBoneTransformBuffer.cleanup();
}