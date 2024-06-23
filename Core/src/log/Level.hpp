#pragma once

#include <string>

namespace gage::log
{
    enum class Level
    {
        None,
        Fatal,
        Error,
        Warn,
        Info,
        Debug,
        Trace,
    };

    std::string get_level_name(Level level);
}