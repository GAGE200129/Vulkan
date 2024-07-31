#pragma once

#include "IComponent.hpp"


#include <string>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <Core/src/gfx/data/GPUBuffer.hpp>
#include <Core/src/gfx/data/CPUBuffer.hpp>
#include <Core/src/gfx/data/Image.hpp>


#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

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
       struct TerrainData
        {
            float min_height{};
            float max_height{};
            float uv_scale{};
        };

        struct Vertex
        {
            glm::vec3 pos{};
            glm::vec2 tex_coord{};
            glm::vec3 normal{};
        };
    public:
        TerrainRenderer(SceneGraph &scene, Node &node, gfx::Graphics& gfx, uint32_t size, uint32_t iteration, float scale, float min_height, float max_height, float filter);
        
        inline void render_imgui() final {};
        inline const char* get_name() const final { return "TerrainRenderer"; };
    private:
        
        gfx::Graphics& gfx;
        uint32_t size{};
        uint32_t iteration{};
        float scale{};
        float min_height{};
        float max_height{};
        float filter{};
        std::vector<float> height_map{};
        std::unique_ptr<gfx::data::GPUBuffer> vertex_buffer{};
        std::unique_ptr<gfx::data::GPUBuffer> index_buffer{};
        std::unique_ptr<gfx::data::Image> image{};

        TerrainData terrain_data{};
        std::unique_ptr<gfx::data::CPUBuffer> terrain_data_buffer{};
        VkDescriptorSet terrain_data_desc{};

        JPH::BodyID height_map_body{};
    };
}