#include <pch.hpp>
#include "hid.hpp"



namespace gage::hid
{
    static std::shared_ptr<spdlog::logger> logger; 
    void init()
    {
        logger = spdlog::stdout_color_mt("Human interface device");
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