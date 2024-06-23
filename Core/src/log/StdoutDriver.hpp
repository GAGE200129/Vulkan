#pragma once

#include "IDriver.hpp"

namespace gage::log
{
    class StdoutDriver : public IDriver
    {
    public:
        void submit(log::Entry &e) override;
    };
}