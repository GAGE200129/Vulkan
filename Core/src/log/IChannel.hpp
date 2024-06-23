#pragma once
#include <memory>
#include <vector>

namespace gage::log
{
    struct Entry;
    class IDriver;
    class IPolicy;

    class IChannel
    {
    public:
        virtual void submit(Entry& ) = 0;
        virtual void attach_driver(std::shared_ptr<IDriver>) = 0;
        virtual void attach_policy(std::shared_ptr<IPolicy>) = 0;
    };

    
}