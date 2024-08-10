#pragma once


#include "ModelNode.hpp"
#include "ModelMesh.hpp"
#include "ModelMaterial.hpp"
#include "ModelAnimation.hpp"

#include <Core/src/gfx/data/GPUBuffer.hpp>
#include <Core/src/gfx/data/CPUBuffer.hpp>
#include <Core/src/gfx/data/Image.hpp>

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gage::scene::data
{
    enum class ModelImportMode
    {
        Binary,
        ASCII
    };
    class Model
    {
    public:
        Model(const gfx::Graphics& gfx, const systems::Renderer& renderer, const std::string &file_path, ModelImportMode mode);
        ~Model();
        

        Model(Model&&) = default;
        Model(const Model&) = delete;
        Model operator=(const Model&) = delete;

    public:
        std::string name{};
        std::vector<ModelNode> nodes{};
        uint32_t root_node{};
        std::vector<ModelMesh> meshes{};
        std::vector<ModelMaterial> materials{};
        std::vector<ModelAnimation> animations{};
        std::vector<std::vector<uint32_t>> skins{};
    };
}