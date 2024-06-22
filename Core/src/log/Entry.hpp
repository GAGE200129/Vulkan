#pragma once

#include <chrono>
#include <string>

#include "Level.hpp"

namespace gage::log
{
    struct Entry 
    {
        Level level_;
        std::string note_;
        const char* file_;
        const char* function_;
        int line_;
        std::chrono::system_clock::time_point timestamp_;
    };
}