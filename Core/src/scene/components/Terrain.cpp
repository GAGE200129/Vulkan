#include <pch.hpp>
#include "Terrain.hpp"

#include "../scene.hpp"
#include <glm/gtc/integer.hpp>

#include <Core/src/gfx/Graphics.hpp>

#include <imgui/imgui.h>

namespace gage::scene::components
{
    Terrain::Terrain(SceneGraph &scene, Node &node,
                                     gfx::Graphics &gfx,
                                     uint32_t patch_count,
                                     uint32_t patch_size,
                                     uint32_t iteration,
                                     float scale,
                                     float min_height, float max_height,
                                     float filter) :

                                                     IComponent(scene, node),
                                                     gfx(gfx),
                                                     size(0),
                                                     patch_count(patch_count),
                                                     patch_size(patch_size),
                                                     scale(scale)
    {
        // calculate size
        {
            this->size = (patch_size - 1) * patch_count + 1;
        }
        // Calculate lod
        {
            max_lod = glm::log2(patch_size - 1);
            lod_infos.resize(max_lod + 1);
            lod_regions.resize(patch_count * patch_count);
        }
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
        generate_vertex_datas();
    }

    void Terrain::generate_vertex_datas()
    {
        // Generate positions and uvs
        for (uint32_t y = 0; y < size; y++)
            for (uint32_t x = 0; x < size; x++)
            {
                float height = height_map.at(x + y * size);
                components::Terrain::Vertex v{};
                v.pos = {x * scale, height, y * scale};
                v.tex_coord = {(float)x / size, (float)y / size};
                v.normal = {0, 1, 0};
                vertex_data.push_back(std::move(v));
            }
        // Calculate number of indices count for each lod
        uint32_t indices_count = 0;
        {
            uint32_t num_quad = (patch_size - 1) * (patch_size - 1);
            const uint32_t indices_per_quad = 6;

            for (uint32_t lod = 0; lod <= this->max_lod; lod++)
            {
                indices_count += num_quad * indices_per_quad;
                num_quad /= 4;
            }
        }
        // Generate indices
        {
            indices_data.resize(indices_count);
            uint32_t index = 0;

            for (uint32_t lod = 0; lod <= this->max_lod; lod++)
            {
                lod_infos.at(lod).start = index;

                const uint32_t step = std::pow(2, lod);
                const uint32_t end_pos = patch_size - 1 - step;

                for (uint32_t y = 0; y <= end_pos; y += step)
                    for (uint32_t x = 0; x <= end_pos; x += step)
                    {
                        uint32_t index_bottom_left = y * size + x;
                        uint32_t index_top_left = (y + step) * size + x;
                        uint32_t index_top_right = (y + step) * size + x + step;
                        uint32_t index_bottom_right = y * size + x + step;

                        indices_data.at(index++) = index_bottom_left;
                        indices_data.at(index++) = index_top_left;
                        indices_data.at(index++) = index_top_right;

                        indices_data.at(index++) = index_bottom_left;
                        indices_data.at(index++) = index_top_right;
                        indices_data.at(index++) = index_bottom_right;
                    }

                lod_infos.at(lod).count = index - lod_infos.at(lod).start;
            }
        }
        // Calculate normals
        {

            for (uint32_t y = 0; y < (size - 1); y += (patch_size - 1))
                for (uint32_t x = 0; x < (size - 1); x += (patch_size - 1))
                {
                    uint32_t base_vertex = x + size * y;

                    for (uint32_t i = 0; i < indices_data.size(); i += 3)
                    {
                        uint32_t i0 = base_vertex + indices_data.at(i + 0);
                        uint32_t i1 = base_vertex + indices_data.at(i + 1);
                        uint32_t i2 = base_vertex + indices_data.at(i + 2);
                        glm::vec3 v1 = vertex_data.at(i1).pos - vertex_data.at(i0).pos;
                        glm::vec3 v2 = vertex_data.at(i2).pos - vertex_data.at(i0).pos;
                        glm::vec3 n = glm::normalize(glm::cross(v1, v2));

                        vertex_data.at(i0).normal += n;
                        vertex_data.at(i1).normal += n;
                        vertex_data.at(i2).normal += n;
                    }
                }

            for (auto &v : vertex_data)
            {
                v.normal = glm::normalize(v.normal);
            }
        }
    }

    Terrain::Terrain(SceneGraph &scene, Node &node, gfx::Graphics &gfx, uint32_t patch_count, uint32_t patch_size, float scale) :
        IComponent(scene, node),
        gfx(gfx),
        size(0),
        patch_count(patch_count),
        patch_size(patch_size),
        scale(scale)
    {
        // calculate size
        {
            this->size = (patch_size - 1) * patch_count + 1;
        }
        // Calculate lod
        {
            max_lod = glm::log2(patch_size - 1);
            lod_infos.resize(max_lod + 1);
            lod_regions.resize(patch_count * patch_count);
        }
        // Generate height map
        {
            // Default it to zero
            height_map.reserve(size * size);
            for (uint32_t i = 0; i < (size * size); i++)
            {
                height_map.push_back(0.0f);
            }
        }

        
        generate_vertex_datas();
    }

    uint32_t Terrain::get_current_lod(uint32_t patch_x, uint32_t patch_y)
    {
        return lod_regions.at(patch_x + this->patch_count * patch_y);
    }

    void Terrain::render_imgui()
    {
        
    }

    nlohmann::json Terrain::to_json() const
    {
        return {};
    }

    bool Terrain::is_inside_frustum(uint32_t x, uint32_t y, const glm::mat4x4 &view, const glm::mat4x4 &proj)
    {
        auto test = [](const glm::vec3 &p, const glm::mat4x4 &view, const glm::mat4x4 &proj) -> bool
        {
            glm::vec4 p_4d(p, 1.0f);
            glm::vec4 clip = proj * view * p_4d;
            bool inside = (clip.x <= clip.w) && (clip.x >= -clip.w) &&
                          (clip.y <= clip.w) && (clip.y >= -clip.w) &&
                          (clip.z <= clip.w) && (clip.z >= -clip.w);
            return inside;
        };
        uint32_t x0 = x;
        uint32_t x1 = x + patch_size - 1;
        uint32_t y0 = y;
        uint32_t y1 = y + patch_size - 1;

        glm::vec3 p00 = {float(x0) * scale, height_map.at(x0 + size * y0), float(y0) * scale};
        glm::vec3 p01 = {float(x0) * scale, height_map.at(x0 + size * y1), float(y1) * scale};
        glm::vec3 p10 = {float(x1) * scale, height_map.at(x1 + size * y0), float(y0) * scale};
        glm::vec3 p11 = {float(x1) * scale, height_map.at(x1 + size * y1), float(y1) * scale};

        bool inside_view_frustum = test(p00, view, proj) || test(p01, view, proj) || test(p10, view, proj) || test(p11, view, proj);
        return inside_view_frustum;
    }

    void Terrain::update_lod_regons(const glm::vec3 &camera_position)
    {
        const uint32_t center_step = patch_size / 2;
        for (uint32_t patch_y = 0; patch_y < patch_count; patch_y++)
            for (uint32_t patch_x = 0; patch_x < patch_count; patch_x++)
            {
                uint32_t x = patch_x * (patch_size - 1) + center_step;
                uint32_t y = patch_y * (patch_size - 1) + center_step;

                glm::vec3 patch_center = {(float)x * scale, 0.0f, (float)y * scale};

                float distance = glm::length(patch_center - glm::vec3{camera_position.x, 0.0f, camera_position.z});
                uint32_t lod = max_lod;

                if (distance < 50.0f)
                    lod = 0;
                else if (distance < 100.0f)
                    lod = 1;
                else if (distance < 150.0f)
                    lod = 2;
                else if (distance < 200.0f)
                    lod = 3;

                lod_regions.at(patch_x + this->patch_count * patch_y) = lod;
            }
    }
}