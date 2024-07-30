#include <pch.hpp>
#include "Terrain.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TerrainPipeline.hpp"
#include "../../Graphics.hpp"

namespace gage::gfx::data::terrain
{
    Terrain::Terrain(Graphics &gfx, const std::string &file_name, float scale, float height_scale) :
        gfx(gfx)
    {
        std::vector<float> height_map{};

        auto binary = utils::file_path_to_binary(file_name);

        size = glm::sqrt(binary.size() / sizeof(float));
        height_map.resize(size * size);
        std::cout << binary.size() << " | " << height_map.size() << "\n";
        std::memcpy(height_map.data(), binary.data(), binary.size());

        // Populate vertex buffer

        std::vector<Vertex> vertex_data;
        std::vector<uint32_t> indices_data;

        uint32_t index = 0;
        for (uint32_t y = 0; y < size; y++)
            for (uint32_t x = 0; x < size; x++)
            {
                float height = height_map.at(index++) * height_scale;
                vertex_data.push_back({{x * scale, height, y * scale}});
            }

        uint32_t num_quad = (size - 1) * (size - 1);
        indices_data.resize(num_quad * 6);
        index = 0;
        for (uint32_t y = 0; y < (size - 1); y++)
            for (uint32_t x = 0; x < (size - 1); x++)
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

    Terrain::Terrain(Graphics &gfx, uint32_t size, uint32_t iteration, float scale, float min_height, float max_height, float filter) :
        gfx(gfx)
    {
        this->size = size;
        // Generate height map
        std::vector<float> height_map{};
        {
            // Default it to zero
            height_map.reserve(size * size);
            for (uint32_t i = 0; i < (size * size); i++)
            {
                height_map.push_back(0.0f);
            }
        }

        // Fault formation
        {
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<uint32_t> dist(0, size);

            float delta_height = max_height - min_height;
            for (uint32_t current_iter = 0; current_iter < iteration; current_iter++)
            {
                float ratio = (float)current_iter / (float)iteration;
                float height = max_height - ratio * delta_height;
                
                
                // Pick random terrain point
                glm::ivec2 p1, p2;
                p1.x = dist(rng);
                p1.y = dist(rng);

                // Make sure they're not the same point
                do
                {
                    p2.x = dist(rng);
                    p2.y = dist(rng);
                } while (p1 == p2);

                

                glm::ivec2 delta = p2 - p1;

                for (uint32_t y = 0; y < size; y++)
                    for (uint32_t x = 0; x < size; x++)
                    {
                        glm::ivec2 curr_vec = {x - p1.x, y - p1.y};

                        int cross = curr_vec.x * delta.y - delta.x * curr_vec.y;
                        //log().info("curr_vec: {}, delta: {}, cross: {}", glm::to_string(curr_vec), glm::to_string(delta), cross);
                        if(cross > 0)
                        {
                            height_map.at(x + size * y) += height;
                            
                        }
                    }
            }
        }

        //Apply fir filter
        {

            auto fir_filter_single_point = [&height_map, this](uint32_t x, uint32_t y, float pre_val, float filter)
            {
                float current_val = height_map.at(x + y * this->size);
                float new_val = filter * pre_val + (1.0 - filter) * current_val;

                height_map.at(x + y * this->size) = new_val;
                return new_val;
            };
            //left to right
            for(uint32_t y = 0; y < size; y++)
            {
                float prev_val = height_map.at(0 + y * size);
                for(uint32_t x = 1; x < size; x++)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }

            //right to left
            for(uint32_t y = 0; y < size; y++)
            {
                float prev_val = height_map.at((size - 1) + y * size);
                for(int32_t x = size - 2; x >= 0; x--)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }

            //Bottom to top
            for(uint32_t x = 0; x < size; x++)
            {
                float prev_val = height_map.at(x + 0 * size);
                for(uint32_t y = 1; y < size; y++)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }

            //Top to bottom
            for(uint32_t x = 0; x < size; x++)
            {
                float prev_val = height_map.at(x + (size - 1) * size);
                for(int32_t y = size - 2; y >= 0; y--)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }
        }

        //Normalize the heightmap
        {
            //Find min max of current height map
            float current_min_height = std::numeric_limits<float>::max();
            float current_max_height = std::numeric_limits<float>::lowest();

            for(const float& height : height_map)
            {
                if(height > current_max_height)
                    current_max_height = height;
                if(height < current_min_height)
                    current_min_height = height;
            }

            float delta = current_max_height - current_min_height;
            float range = max_height - min_height;
            for(float& height : height_map)
            {
                height = ((height - current_min_height) / delta) * range + min_height;
            }   
        }

        // Generate triangle list and upload it to vulkan
        {
            std::vector<Vertex> vertex_data;
            std::vector<uint32_t> indices_data;

            for (uint32_t y = 0; y < size; y++)
                for (uint32_t x = 0; x < size; x++)
                {
                    float height = height_map.at(x + y * size);
                    
                    vertex_data.push_back({{x * scale, height, y * scale}});
                }
            uint32_t num_quad = (size - 1) * (size - 1);
            indices_data.resize(num_quad * 6);
            uint32_t index = 0;
            for (uint32_t y = 0; y < (size - 1); y++)
                for (uint32_t x = 0; x < (size - 1); x++)
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

        //Create descriptor
        {
            terrain_data.min_height = min_height;
            terrain_data.max_height = max_height;

        
            terrain_data_buffer = std::make_unique<CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(TerrainData), nullptr);
            terrain_data_desc = gfx.get_terrain_pipeline().allocate_descriptor_set(sizeof(TerrainData), terrain_data_buffer->get_buffer_handle());
        }
    }

    Terrain::~Terrain()
    {
        gfx.get_terrain_pipeline().free_descriptor_set(terrain_data_desc);
    }
    void Terrain::render(Graphics &gfx, VkCommandBuffer cmd)
    {
        std::memcpy(terrain_data_buffer->get_mapped(), &terrain_data, sizeof(TerrainData));
        VkBuffer buffers[] =
            {
                vertex_buffer->get_buffer_handle()};
        VkDeviceSize offsets[] = {
            0};
        glm::mat4x4 transform(1.0f);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx.get_terrain_pipeline().get_layout(), 1, 1, &terrain_data_desc, 0, nullptr);
        vkCmdPushConstants(cmd, gfx.get_terrain_pipeline().get_layout(), VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(transform));
        vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(cmd, index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, (size - 1) * (size - 1) * 6, 1, 0, 0, 0);
    }
}