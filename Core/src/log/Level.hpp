#pragma once

#include <string>

namespace gage::log
{
    enum class Level
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    std::string get_level_name(Level level);
}