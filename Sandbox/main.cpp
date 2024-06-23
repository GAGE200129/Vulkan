#include <stdio.h>

#include <iostream>

#include <Core/src/log/EntryBuilder.hpp>
#include <Core/src/log/IChannel.hpp>
#include <Core/src/log/DefaultChannel.hpp>
#include <Core/src/log/IDriver.hpp>
#include <Core/src/log/IPolicy.hpp>
#include <Core/src/log/ServerityLevelPolicy.hpp>

using namespace gage;

#define dumb_logger log::EntryBuilder{__FILE__, __FUNCTION__, __LINE__}.channel(&channel)

class DumbChannel : public log::IChannel
{
public:
    void submit(log::Entry &e) override
    {
        entry_ = e;
    }

    void attach_driver(std::shared_ptr<log::IDriver> driver) override {}
    void attach_policy(std::unique_ptr<log::IPolicy> policy) override {}

    log::Entry entry_;
};

class StdOutDriver : public log::IDriver
{
public:
    void submit(log::Entry &e) override
    {
        std::cout << e.note_ << "\n";
        if(e.trace_)
            std::cout << e.trace_->print() << "\n";
    }
};




void testing3() {
    log::DefaultChannel channel;
    channel.attach_driver(std::make_shared<StdOutDriver>());
    channel.attach_policy(std::make_unique<log::ServerityLevelPolicy>(log::Level::Info));
    dumb_logger.fatal("Fatal");
    dumb_logger.warn("Warn");
    dumb_logger.error("Error");
}

void testing2() {
    testing3();
}

void testing() {
    testing2();
}


int main()
{
    testing();
    return 0;
}