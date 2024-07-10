#include <pch.hpp>
#include "StackTrace.hpp"

#define BACKWARD_HAS_BFD 1
#include "backward.hpp"

namespace gage::utils
{
    StackTrace::StackTrace()
    {
        p_trace = std::make_unique<backward::StackTrace>();
        p_trace->load_here(32);
    }

    StackTrace::StackTrace(const StackTrace &other)
    {
        p_trace = std::make_unique<backward::StackTrace>(*other.p_trace);
    }
    StackTrace& StackTrace::operator=(const StackTrace &other)
    {
        p_trace = std::make_unique<backward::StackTrace>(*other.p_trace);
        return *this;
    }

    StackTrace::~StackTrace()
    {
    }

    std::string StackTrace::print() const
    {
        std::ostringstream ss;
        backward::Printer printer;
        printer.print(*p_trace, ss);
        return ss.str();
    }
}