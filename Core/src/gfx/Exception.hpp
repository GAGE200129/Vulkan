#pragma once

#include <Core/src/utils/Exception.hpp>

namespace gage::gfx
{
    class GraphicsException : public utils::Exception{ using Exception::Exception; };
}