#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

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
    class Mesh
    {
    public:
        Mesh(Graphics &gfx, const tinygltf::Model& model, const tinygltf::Mesh &mesh);
       ~Mesh();

        void draw(VkCommandBuffer cmd) const;
    private:

        class MeshSection
        {
        public:
            MeshSection(Graphics& gfx,
                const std::vector<uint32_t>& in_index_buffer,
                const std::vector<glm::vec3>& in_position_buffer,
                const std::vector<glm::vec3>& in_normal_buffer,
                const std::vector<glm::vec2>& in_texcoord_buffer);

            uint32_t vertex_count;
            data::GPUBuffer index_buffer; 
            data::GPUBuffer position_buffer; 
            data::GPUBuffer normal_buffer; 
            data::GPUBuffer texcoord_buffer; 
        };

        std::vector<MeshSection> sections{};
    };
}