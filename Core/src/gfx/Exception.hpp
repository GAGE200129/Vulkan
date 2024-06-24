#pragma once

#include <Core/src/utils/Exception.hpp>
#include <Core/src/log/Log.hpp>
#include <vulkan/vulkan.h>
#include <sstream>
#include <string>

#ifndef NDEBUG
#define vk_check(x, ...) {VkResult result = x; if (result != VK_SUCCESS) { logger.fatal(__VA_ARGS__).vk_result(result); throw GraphicsException{}; } }
#define vkb_check(x, ...) { if(!x) { std::stringstream ss; ss << std::string(__VA_ARGS__) << x.error().message(); logger.fatal(ss.str()); throw GraphicsException{};} }
#else
#define vk_check(x, ...) x
#define vkb_check(x, ...)
#endif


namespace gage::gfx
{
    class GraphicsException : public utils::Exception{ using Exception::Exception; };
}