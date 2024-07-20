#pragma once

#include <Core/src/utils/Exception.hpp>
#include <Core/src/utils/StackTrace.hpp>
#include "gfx.hpp"


#ifndef NDEBUG
#define vk_check(x) {VkResult result = x; if (result != VK_SUCCESS) { utils::StackTrace stack_trace; log().critical("{}", stack_trace.print());  throw GraphicsException{}; } }
#define vkb_check(x, ...) { if(!x) { log().critical("{}", x.error().message());  utils::StackTrace stack_trace; log().critical("{}", stack_trace.print()); throw GraphicsException{};} }
#else
#define vk_check(x, ...) x
#define vkb_check(x, ...)
#endif


namespace gage::gfx
{
    class GraphicsException : public utils::Exception{ using Exception::Exception; };
}