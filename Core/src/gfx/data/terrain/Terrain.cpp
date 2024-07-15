#include <pch.hpp>
#include "Terrain.hpp"

#include <Core/src/utils/FileLoader.hpp>

namespace gage::gfx::data::terrain
{
    Terrain::Terrain(Graphics& gfx, const std::string& file_name)
    {
        auto binary = utils::file_path_to_binary(file_name);

        size = binary.size() / sizeof(float);
        height_map.resize(size);
        std::cout << binary.size() << " | " << height_map.size() << "\n";
        std::memcpy(height_map.data(), binary.data(), binary.size());


        struct Vertex
        {
            glm::vec3 pos{};
        };

        //Populate vertex buffer

        std::vector<Vertex> vertex_data;
        std::vector<uint32_t> index_data;

        uint32_t index = 0;
        for(uint32_t y = 0; y < size; y++)
        for(uint32_t x = 0; x < size; x++)
        {
            vertex_data.push_back({{x, 0, y}});
            index++;
        }



        vertex_buffer = std::make_unique<GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_data.size() * sizeof(Vertex), vertex_data.data());
        index_buffer = std::make_unique<GPUBuffer>(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_data.size() * sizeof(uint32_t), index_data.data());
    }

}