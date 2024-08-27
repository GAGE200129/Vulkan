#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include <Core/src/gfx/data/GPUBuffer.hpp>

namespace tinygltf
{
    class Model;
    class Mesh;
}

namespace gage::gfx
{
    class Graphics;
}

namespace gage::scene::data
{
    class ModelMeshPrimitive
    {
    public:
        ModelMeshPrimitive(uint32_t vertex_count, 
            gfx::data::GPUBuffer index_buffer,
            gfx::data::GPUBuffer position_buffer,
            gfx::data::GPUBuffer normal_buffer,
            gfx::data::GPUBuffer texcoord_buffer,
            gfx::data::GPUBuffer bone_id_buffer, 
            gfx::data::GPUBuffer bone_weight_buffer,
            uint32_t material_index,
            bool has_skin) :
            vertex_count(vertex_count),
            index_buffer(std::move(index_buffer)),
            position_buffer(std::move(position_buffer)),
            normal_buffer(std::move(normal_buffer)),
            texcoord_buffer(std::move(texcoord_buffer)),
            bone_id_buffer(std::move(bone_id_buffer)),
            bone_weight_buffer(std::move(bone_weight_buffer)),
            material_index(material_index),
            has_skin(has_skin)
            {}
        ~ModelMeshPrimitive();

        ModelMeshPrimitive(ModelMeshPrimitive&&) = default;
        ModelMeshPrimitive& operator=(ModelMeshPrimitive&&) = default;
        ModelMeshPrimitive(const ModelMeshPrimitive&) = delete;
        ModelMeshPrimitive operator=(const ModelMeshPrimitive&) = delete;
    public:
        uint32_t vertex_count{};
        gfx::data::GPUBuffer index_buffer; 
        gfx::data::GPUBuffer position_buffer; 
        gfx::data::GPUBuffer normal_buffer; 
        gfx::data::GPUBuffer texcoord_buffer;
        gfx::data::GPUBuffer bone_id_buffer;
        gfx::data::GPUBuffer bone_weight_buffer;
        int32_t material_index{};
        bool has_skin{};
    };
    class ModelMesh
    {
    public:
        ModelMesh(const gfx::Graphics& gfx, const tinygltf::Model& model, const tinygltf::Mesh& mesh);
        ~ModelMesh();

        ModelMesh(ModelMesh&&) = default;
        ModelMesh(const ModelMesh&) = delete;
        ModelMesh operator=(const ModelMesh&) = delete;
    public:
        std::vector<ModelMeshPrimitive> primitives{};
        std::vector<uint32_t> skin_joints{};
    };
}