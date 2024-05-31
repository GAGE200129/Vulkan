#pragma once

#include "Components.hpp"
#include "GameObject.hpp"

#include "Asset/StaticModelLoader.hpp"


class StaticModelComponent : public Component
{
public:
    StaticModelComponent(const std::string &filePath) : mFilePath(filePath) {}
    void init() override
    {
        mMeshData = StaticModelLoader::get(mFilePath);
    }

    void render() override;

private:
    StaticModelData *mMeshData;
    std::string mFilePath;
};