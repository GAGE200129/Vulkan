#pragma once

#include <Core/src/utils/Exception.hpp>

namespace spdlog
{
    class logger;
}

namespace gage::scene
{
    class SceneException : public utils::Exception{ using Exception::Exception; };
    
    void init();
    void shutdown();

    spdlog::logger& log();
    
}


