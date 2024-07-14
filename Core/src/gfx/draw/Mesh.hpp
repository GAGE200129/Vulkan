#pragma once

#include "../data/GPUBuffer.hpp"

namespace tinygltf
{
    class Model;
    class Mesh;
}

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::draw
{
    class Model;
    class Mesh
    {
    public:
        Mesh(Graphics &gfx, Model& model, const tinygltf::Model& gltf_model, const tinygltf::Mesh &mesh);
       ~Mesh();


        float get_cull_radius() const;
        void draw(VkCommandBuffer cmd, VkPipelineLayout layout, const glm::mat4x4& transform) const;
    private:
        Graphics& gfx;
        Model& model;
        struct MeshSection
        {
            uint32_t vertex_count;
            std::unique_ptr<data::GPUBuffer> index_buffer; 
            std::unique_ptr<data::GPUBuffer> position_buffer; 
            std::unique_ptr<data::GPUBuffer> normal_buffer; 
            std::unique_ptr<data::GPUBuffer> texcoord_buffer;
            int32_t material_index;
        };
        float cull_radius{};
        std::vector<MeshSection> sections{};
    };
}