#pragma once

#include "Level.hpp"

#include <Core/src/utils/StackTrace.hpp>

#include <chrono>
#include <string>
#include <optional>


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
        std::optional<utils::StackTrace> trace_;
    };
}