#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Model::init(const std::string &fileName)
{
  Assimp::Importer importer;

  const aiScene *pScene = importer.ReadFile(fileName,
                            aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

  if (pScene == nullptr)
  {
    throw std::runtime_error(importer.GetErrorString());
  }
  //Count and reserve vertex data for all meshes in scene
  unsigned int numVertices = 0, numIndices = 0;
  mMeshes.resize(pScene->mNumMeshes);
  for(unsigned int i = 0; i < pScene->mNumMeshes; i++)
  {
    mMeshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
    mMeshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
    mMeshes[i].baseVertex = numVertices;
    mMeshes[i].baseIndex = numIndices;

    numVertices += pScene->mMeshes[i]->mNumVertices;
    numIndices += mMeshes[i].numIndices;
  }
  mPositions.reserve(numVertices);
  mNormals.reserve(numVertices);
  mUvs.reserve(numVertices);
  mIndices.reserve(numIndices);

  //Populate mesh datas
  for(unsigned int i = 0; i < pScene->mNumMeshes; i++)
  {
    const aiMesh* mesh = pScene->mMeshes[i];

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      const aiVector3D& pos = mesh->mVertices[i];
      const aiVector3D& normal = mesh->mNormals[i];
      const aiVector3D& uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0, 0, 0);

      mPositions.push_back({pos.x, pos.y, pos.z});
      mNormals.push_back({normal.x, normal.y, normal.z});
      mUvs.push_back({uv.x, uv.y});
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      const aiFace& face = mesh->mFaces[i];
      if(face.mNumIndices != 3)
        throw std::runtime_error("Wrong face type !");
      mIndices.push_back(face.mIndices[0]);
      mIndices.push_back(face.mIndices[1]);
      mIndices.push_back(face.mIndices[2]);
    }
  }

  //Push mesh data to vulkan buffer
  mPositionBuffer.initAndTransferToLocalDevice(mPositions.data(), 
  mPositions.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  mNormalBuffer.initAndTransferToLocalDevice(mNormals.data(), 
  mNormals.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  mUvBuffer.initAndTransferToLocalDevice(mUvs.data(), 
  mUvs.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);

  mIndexBuffer.initAndTransferToLocalDevice(mIndices.data(),
  mIndices.size() * sizeof(unsigned int), vk::BufferUsageFlagBits::eIndexBuffer);


}
