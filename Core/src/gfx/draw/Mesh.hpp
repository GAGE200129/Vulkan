#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <memory>

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

        void draw(VkCommandBuffer cmd) const;
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

        std::vector<MeshSection> sections{};
    };
}