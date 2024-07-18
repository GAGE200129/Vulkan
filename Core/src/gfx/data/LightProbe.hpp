#pragma once

#include "GPUBuffer.hpp"

#include <memory>

namespace gage::gfx::data
{
    class LightProbe
    {
    public:
        LightProbe();
        ~LightProbe();
    private:
        std::unique_ptr<GPUBuffer> buffer;
    };
}