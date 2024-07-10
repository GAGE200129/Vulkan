#pragma once

namespace backward
{
    class StackTrace;
}

namespace gage::utils
{
    class StackTrace
    {
    public:
        StackTrace();
        StackTrace(const StackTrace&);
        StackTrace& operator=(const StackTrace&);
        ~StackTrace();

        std::string print() const;
    private:
        std::unique_ptr<backward::StackTrace> p_trace;
    };
}