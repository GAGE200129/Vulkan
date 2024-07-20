#include <pch.hpp>
#include "Terrain.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TerrainPipeline.hpp"
#include "../../Graphics.hpp"

namespace gage::gfx::data::terrain
{
    Terrain::Terrain(Graphics& gfx, const std::string& file_name)
    {
        auto binary = utils::file_path_to_binary(file_name);

        size = glm::sqrt(binary.size() / sizeof(float));
        height_map.resize(size * size);
        std::cout << binary.size() << " | " << height_map.size() << "\n";
        std::memcpy(height_map.data(), binary.data(), binary.size());
        struct Vertex
        {
            glm::vec3 pos{};
        };

        //Populate vertex buffer

        std::vector<Vertex> vertex_data;
        std::vector<uint32_t> indices_data;

        uint32_t index = 0;
        for(uint32_t y = 0; y < size; y++)
            for(uint32_t x = 0; x < size; x++)
            {
                vertex_data.push_back({{x * 10.0f, height_map.at(index++) * 0.6f, y * 10.0f}});

            }

        uint32_t num_quad = (size - 1) * (size - 1);
        indices_data.resize(num_quad * 6);
        index = 0;
        for(uint32_t y = 0; y < (size - 1); y++)
            for(uint32_t x = 0; x < (size - 1); x++)
            {
                uint32_t index_bottom_left = y * size + x;
                uint32_t index_top_left = (y + 1) * size + x;
                uint32_t index_top_right = (y + 1) * size + x + 1;
                uint32_t index_bottom_right = y * size + x + 1;

                indices_data[index++] = index_bottom_left;
                indices_data[index++] = index_top_left;
                indices_data[index++] = index_top_right;

                indices_data[index++] = index_bottom_left;
                indices_data[index++] = index_top_right;
                indices_data[index++] = index_bottom_right;
            }



        vertex_buffer = std::make_unique<GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_data.size() * sizeof(Vertex), vertex_data.data());
        index_buffer = std::make_unique<GPUBuffer>(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices_data.size() * sizeof(uint32_t), indices_data.data());
    }
    void Terrain::render(Graphics& gfx, VkCommandBuffer cmd)
    {
        VkBuffer buffers[] = 
        {
            vertex_buffer->get_buffer_handle()
        };
        VkDeviceSize offsets[] = {
            0
        };
        glm::mat4x4 transform(1.0f);
        vkCmdPushConstants(cmd, gfx.get_terrain_pipeline().get_layout(), VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(transform));
        vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(cmd, index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, (size - 1) * (size - 1) * 6, 1, 0, 0, 0);
    }
}