#include <pch.hpp>
#include "scene.hpp"



namespace gage::scene
{
    static std::shared_ptr<spdlog::logger> logger; 
    void init()
    {
        logger = spdlog::stdout_color_mt("Scene");
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