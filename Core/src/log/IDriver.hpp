#pragma once


namespace gage::log
{
    struct Entry;
    class IDriver
    {
    public:
        virtual void submit(Entry& ) = 0;
    };


}