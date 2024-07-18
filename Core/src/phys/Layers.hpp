#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Core/NonCopyable.h>

namespace gage::phys
{
    namespace Layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    };
}