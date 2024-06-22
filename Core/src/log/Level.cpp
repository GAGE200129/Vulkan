#include "Level.hpp"

namespace gage::log
{

    std::string get_level_name(Level level)
    {
        switch(level)
        {
            case Level::Trace: return "Trace";
            case Level::Debug: return "Debug";
            case Level::Info: return "Info";
            case Level::Warn: return "Warn";
            case Level::Error: return "Error";
            case Level::Fatal: return "Fatal";
            default: return "Unknown";
            
        };
    }
}