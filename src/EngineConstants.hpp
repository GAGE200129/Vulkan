#pragma once

namespace EngineConstants
{
    constexpr double TICK_TIME = 1.0 / 60.0;

    #ifdef DEBUG
    static constexpr bool DEBUG_ENABLED = true;
    #else
    static constexpr bool DEBUG_ENABLED = false;
    #endif
}