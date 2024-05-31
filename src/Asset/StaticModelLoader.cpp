#include "pch.hpp"
#include "StaticModelLoader.hpp"

#include <stb/stb_image.h>

namespace StaticModelLoader
{
    std::map<std::string, std::unique_ptr<StaticModelData>> sCache;
}

StaticModelData *StaticModelLoader::get(const std::string &filePath)
{
    if (sCache.find(filePath) != sCache.end())
    {
        return sCache.at(filePath).get();
    }
    else
    {
        return initCache(filePath);
    }
}

StaticModelData *StaticModelLoader::initCache(const std::string &filePath)
{
    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(filePath,
                                              aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (pScene == nullptr)
    {
        throw std::runtime_error(importer.GetErrorString());
    }

    spdlog::info("Caching model: {}", filePath);
    auto meshData = std::make_unique<StaticModelData>();
    // Count and reserve vertex data for all meshes in scene
    unsigned int numVertices = 0, numIndices = 0, numBones = 0;
    meshData->mMeshes.resize(pScene->mNumMeshes);
    for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
    {
        meshData->mMeshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
        meshData->mMeshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;
        meshData->mMeshes[i].baseVertex = numVertices;
        meshData->mMeshes[i].baseIndex = numIndices;

        numVertices += pScene->mMeshes[i]->mNumVertices;
        numIndices += meshData->mMeshes[i].numIndices;
        numBones += pScene->mMeshes[i]->mNumBones;
    }

    meshData->mPositions.reserve(numVertices);
    meshData->mNormals.reserve(numVertices);
    meshData->mUvs.reserve(numVertices);
    meshData->mIndices.reserve(numIndices);

    populateMeshData(meshData.get(), pScene);
    populateTextures(meshData.get(), pScene);
    // Push mesh data to vulkan buffer

    VulkanEngine::bufferInitAndTransferToLocalDevice(meshData->mPositions.data(),
                meshData->mPositions.size() * sizeof(glm::vec3),
                vk::BufferUsageFlagBits::eVertexBuffer,
                meshData->mPositionBuffer);
    
    VulkanEngine::bufferInitAndTransferToLocalDevice(meshData->mUvs.data(),
                meshData->mUvs.size() * sizeof(glm::vec2),
                vk::BufferUsageFlagBits::eVertexBuffer,
                meshData->mUvBuffer);

    VulkanEngine::bufferInitAndTransferToLocalDevice(meshData->mNormals.data(),
                meshData->mNormals.size() * sizeof(glm::vec3),
                vk::BufferUsageFlagBits::eVertexBuffer,
                meshData->mNormalBuffer);

    VulkanEngine::bufferInitAndTransferToLocalDevice(meshData->mIndices.data(),
                meshData->mIndices.size() * sizeof(unsigned int),
                vk::BufferUsageFlagBits::eIndexBuffer,
                meshData->mIndexBuffer);
    
    auto ptr = meshData.get();
    sCache[filePath] = std::move(meshData);

    return ptr;
}

bool StaticModelLoader::populateMeshData(StaticModelData *meshData, const aiScene *scene)
{
    // Populate mesh datas
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        const aiMesh *mesh = scene->mMeshes[i];

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
            {
                spdlog::critical("Face must be triangle !");
                return false;
            }
            meshData->mIndices.push_back(face.mIndices[0]);
            meshData->mIndices.push_back(face.mIndices[1]);
            meshData->mIndices.push_back(face.mIndices[2]);
        }
    }

    return true;
}

bool StaticModelLoader::populateTextures(StaticModelData *meshData, const aiScene *scene)
{
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        const auto *aiMaterial = scene->mMaterials[i];
        // Check for assimp's default material
        if (aiMaterial->GetName().length == 0)
            continue;

        aiString textureFile;
        aiMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureFile);
        const aiTexture *texture = scene->GetEmbeddedTexture(textureFile.C_Str());
        if (!texture)
        {
            spdlog::critical("Model component only accepts embedded textures !");
            return false;
        }

        int width = texture->mWidth, height = texture->mHeight;
        stbi_uc *imageData = (stbi_uc *)texture->pcData;

        // Raw texture data, need a decoder
        bool needDecoder = false;
        if (texture->mHeight == 0)
        {
            needDecoder = true;
            int bpp;
            imageData = stbi_load_from_memory((stbi_uc *)texture->pcData, texture->mWidth, &width, &height, &bpp, STBI_rgb_alpha);
            if (!imageData)
            {
                spdlog::critical("Something wrong with encoded embedded texture !");
                return false;
            }
        }
        meshData->mMaterials.emplace_back();
        VulkanTexture &diffuseTexture = meshData->mMaterials.back().mDiffuse;
        size_t imageSize = width * height * 4;
        // Create a staging buffer and copy to it
        VulkanBuffer staging;
        VulkanEngine::bufferInit(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     staging);
        VulkanEngine::bufferCopy(imageData, imageSize, staging);

        // Create image handle
        VulkanEngine::textureInit(width, height,
                                  vk::Format::eR8G8B8A8Srgb,
                                  vk::ImageTiling::eOptimal,
                                  vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                  vk::MemoryPropertyFlagBits::eDeviceLocal, VulkanEngine::gData.staticModelPipeline.imageDescriptorLayout,
                                  diffuseTexture);

        // Upload image to GPU
        VulkanEngine::textureTransitionLayout(diffuseTexture.handle, vk::Format::eR8G8B8Srgb,
                                              vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandPool(VulkanEngine::gData.commandPool)
            .setCommandBufferCount(1);

        auto commandBuffers = VulkanEngine::gData.device.allocateCommandBuffers(allocInfo).value;
        vk::CommandBuffer commandBuffer = commandBuffers[0];
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        commandBuffer.begin(beginInfo);

        vk::BufferImageCopy region;
        region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLayerCount(1);
        region.setImageOffset(vk::Offset3D(0, 0, 0))
            .setImageExtent(vk::Extent3D(width, height, 1));

        commandBuffer.copyBufferToImage(staging.buffer, diffuseTexture.handle, vk::ImageLayout::eTransferDstOptimal, {region});
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffer);
        VulkanEngine::gData.graphicQueue.submit(submitInfo);
        VulkanEngine::gData.graphicQueue.waitIdle();
        VulkanEngine::gData.device.freeCommandBuffers(VulkanEngine::gData.commandPool, commandBuffer);

        VulkanEngine::textureTransitionLayout(diffuseTexture.handle, vk::Format::eR8G8B8Srgb,
                                              vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        VulkanEngine::bufferCleanup(staging);

        if (needDecoder)
            stbi_image_free(imageData);
    }

    return true;
}

void StaticModelLoader::clearCache()
{
    for (auto &[path, mesh] : sCache)
    {
        VulkanEngine::bufferCleanup(mesh->mPositionBuffer);
        VulkanEngine::bufferCleanup(mesh->mNormalBuffer);
        VulkanEngine::bufferCleanup(mesh->mUvBuffer);
        VulkanEngine::bufferCleanup(mesh->mIndexBuffer);

        for (auto &material : mesh->mMaterials)
        {
            VulkanEngine::textureCleanup(material.mDiffuse);
        }
    }
    sCache.clear();
}
