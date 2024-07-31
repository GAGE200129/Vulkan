#include <pch.hpp>
#include "TerrainRenderer.hpp"

#include "../scene.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/terrain/TerrainPipeline.hpp>

namespace gage::scene::components
{
    TerrainRenderer::TerrainRenderer(SceneGraph &scene, Node &node,
         gfx::Graphics &gfx,
         uint32_t size,
         uint32_t iteration,
         float scale,
         float min_height, float max_height,
         float filter) : 
         
        IComponent(scene, node),
        gfx(gfx),
        size(size),
        iteration(iteration),
        scale(scale),
        min_height(min_height),
        max_height(max_height),
        filter(filter)
    {
        // Generate height map
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
                        // log().info("curr_vec: {}, delta: {}, cross: {}", glm::to_string(curr_vec), glm::to_string(delta), cross);
                        if (cross > 0)
                        {
                            height_map.at(x + size * y) += height;
                        }
                    }
            }
        }

        // Apply fir filter
        {

            auto fir_filter_single_point = [this](uint32_t x, uint32_t y, float pre_val, float filter)
            {
                float current_val = height_map.at(x + y * this->size);
                float new_val = filter * pre_val + (1.0 - filter) * current_val;

                height_map.at(x + y * this->size) = new_val;
                return new_val;
            };
            // left to right
            for (uint32_t y = 0; y < size; y++)
            {
                float prev_val = height_map.at(0 + y * size);
                for (uint32_t x = 1; x < size; x++)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }

            // right to left
            for (uint32_t y = 0; y < size; y++)
            {
                float prev_val = height_map.at((size - 1) + y * size);
                for (int32_t x = size - 2; x >= 0; x--)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }

            // Bottom to top
            for (uint32_t x = 0; x < size; x++)
            {
                float prev_val = height_map.at(x + 0 * size);
                for (uint32_t y = 1; y < size; y++)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }

            // Top to bottom
            for (uint32_t x = 0; x < size; x++)
            {
                float prev_val = height_map.at(x + (size - 1) * size);
                for (int32_t y = size - 2; y >= 0; y--)
                {
                    prev_val = fir_filter_single_point(x, y, prev_val, filter);
                }
            }
        }

        // Normalize the heightmap
        {
            // Find min max of current height map
            float current_min_height = std::numeric_limits<float>::max();
            float current_max_height = std::numeric_limits<float>::lowest();

            for (const float &height : height_map)
            {
                if (height > current_max_height)
                    current_max_height = height;
                if (height < current_min_height)
                    current_min_height = height;
            }

            float delta = current_max_height - current_min_height;
            float range = max_height - min_height;
            for (float &height : height_map)
            {
                height = ((height - current_min_height) / delta) * range + min_height;
            }
        }
    }
}