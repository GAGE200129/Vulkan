#pragma once
#include <memory>
#include <vector>

namespace gage::log
{
    struct Entry;
    class IDriver;
    class IChannel
    {
    public:
        virtual void submit(Entry& ) = 0;
        virtual void attach_driver(std::shared_ptr<IDriver>) = 0;
    };

    class Channel : public IChannel
    {
    public:
        Channel(std::vector<std::shared_ptr<IDriver>> = {});

        virtual void submit(Entry& ) override;
        virtual void attach_driver(std::shared_ptr<IDriver>) override;
    private:
        std::vector<std::shared_ptr<IDriver>> driver_ptrs;
    };
}