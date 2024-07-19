#pragma once

namespace spdlog
{
    class logger;
}

namespace gage::gfx
{
    void init();
    void shutdown();

    spdlog::logger& get_logger();
 
}

#define log gage::gfx::get_logger()

