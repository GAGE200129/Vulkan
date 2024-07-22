#pragma once

namespace spdlog
{
    class logger;
}

namespace gage::scene
{
    void init();
    void shutdown();

    spdlog::logger& log();
    
}


