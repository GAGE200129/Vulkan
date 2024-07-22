#include <pch.hpp>
#include "scene.hpp"



namespace gage::scene
{
    static std::shared_ptr<spdlog::logger> logger; 
    void init()
    {
        logger = spdlog::stdout_color_mt("Scene");
        logger->set_level(spdlog::level::trace);
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