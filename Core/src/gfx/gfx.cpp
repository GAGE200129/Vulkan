#include <pch.hpp>
#include "gfx.hpp"



namespace gage::gfx
{
    static std::shared_ptr<spdlog::logger> logger; 
    void init()
    {
        logger = spdlog::stdout_color_mt("Graphics");
    }
    void shutdown()
    {
        logger.reset();
    }

    spdlog::logger& get_logger()
    {
        return *logger;
    }
}