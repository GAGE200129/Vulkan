#include "StdoutDriver.hpp"

#include "Entry.hpp"

#include <iostream>

namespace gage::log
{
    void StdoutDriver::submit(log::Entry &e)
    {
        std::cout << e.note_ << "\n";
        if (e.trace_)
            std::cout << e.trace_->print() << "\n";
    }
}