#pragma once

namespace spdlog
{
    class logger;
}

namespace gage::gfx
{
    void init();
    void shutdown();

    spdlog::logger& log();
 
}


