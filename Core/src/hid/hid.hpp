#pragma once

namespace spdlog
{
    class logger;
}

namespace gage::hid
{
    void init();
    void shutdown();

    spdlog::logger& log();
    
}


