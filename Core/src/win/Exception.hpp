#pragma once

#include <Core/src/utils/Exception.hpp>

namespace gage::win
{
    class WindowException : public utils::Exception{ using Exception::Exception; };
}