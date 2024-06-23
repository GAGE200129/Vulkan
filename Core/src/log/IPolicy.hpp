#pragma once


namespace gage::log
{
    struct Entry;
    class IPolicy
    {
    public:
        virtual ~IPolicy() = default;
        virtual bool transform_filter(Entry& ) = 0;
    };


}