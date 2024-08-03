#pragma once

#include "IComponent.hpp"

#include <string>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>



namespace gage::scene::systems
{
    class Renderer;
    class Physics;
}

namespace gage::scene::components
{
    class TerrainRenderer final : public IComponent
    {
        friend class systems::Renderer;
        friend class systems::Physics;

    public:
        struct Vertex
        {
            glm::vec3 pos{};
            glm::vec2 tex_coord{};
            glm::vec3 normal{};
        };

        struct LODInfo
        {
            uint32_t start{};
            uint32_t count{};
        };

    public:
        TerrainRenderer(SceneGraph &scene, Node &node, gfx::Graphics &gfx, uint32_t patch_count, uint32_t patch_size, uint32_t iteration, float scale, float min_height, float max_height, float filter);
        TerrainRenderer(SceneGraph &scene, Node &node, gfx::Graphics &gfx, uint32_t patch_count, uint32_t patch_size, float scale);
        

        nlohmann::json to_json() const final;

        void update_lod_regons(const glm::vec3& camera_position);
        bool is_inside_frustum(uint32_t x, uint32_t y, const glm::mat4x4& view, const glm::mat4x4& proj);

        uint32_t get_current_lod(uint32_t patch_x, uint32_t patch_y);

        void render_imgui() final;
        inline const char *get_name() const final { return "TerrainRenderer"; };

    private:
        void generate_vertex_datas();
        
    private:
        gfx::Graphics &gfx;
        uint32_t size{};
        uint32_t patch_size{};
        uint32_t patch_count{};
        uint32_t max_lod{};
        std::vector<LODInfo> lod_infos{};
        std::vector<uint32_t> lod_regions{};
        float scale{};
        std::vector<float> height_map{};
        std::vector<Vertex> vertex_data;
        std::vector<uint32_t> indices_data;
    };
}