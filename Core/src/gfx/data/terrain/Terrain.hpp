#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../GPUBuffer.hpp"
#include "../CPUBuffer.hpp"

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::terrain
{
    class Terrain
    { 
        struct Vertex
        {
            glm::vec3 pos{};
        };

        struct TerrainData
        {
            float min_height{};
            float max_height{};
        };
    public:
        Terrain(Graphics& gfx, const std::string& file_name, float scale, float height_scale);
        Terrain(Graphics& gfx, uint32_t size, uint32_t iteration, float scale, float min_height, float max_height, float filter);
        ~Terrain();

        void render(Graphics& gfx, VkCommandBuffer cmd);

    private:
        gfx::Graphics& gfx;
        uint32_t size{};
        std::unique_ptr<GPUBuffer> vertex_buffer{};
        std::unique_ptr<GPUBuffer> index_buffer{};

        TerrainData terrain_data{};
        std::unique_ptr<CPUBuffer> terrain_data_buffer{};
        VkDescriptorSet terrain_data_desc{};
    };
}