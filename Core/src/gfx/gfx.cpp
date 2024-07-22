#include <pch.hpp>
#include "gfx.hpp"



namespace gage::gfx
{
    static std::shared_ptr<spdlog::logger> logger; 
    void init()
    {
        logger = spdlog::stdout_color_mt("Graphics");
        logger->set_level(spdlog::level::info);
    }
    void shutdown()
    {
        logger.reset();
    }

    spdlog::logger& log()
    {
        return *logger;
    }
}