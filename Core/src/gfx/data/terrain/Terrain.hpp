#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../GPUBuffer.hpp"

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::terrain
{
    class Terrain
    {  
    public:
        Terrain(Graphics& gfx, const std::string& file_name);
    protected:

        uint32_t size{};
        std::vector<float> height_map{};

        std::unique_ptr<GPUBuffer> vertex_buffer{};
        std::unique_ptr<GPUBuffer> index_buffer{};
    };
}