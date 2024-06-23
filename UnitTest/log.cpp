#include <catch2/catch.hpp>

#include <Core/src/log/EntryBuilder.hpp>
#include <Core/src/log/IChannel.hpp>
#include <Core/src/log/IDriver.hpp>
#include <Core/src/log/IPolicy.hpp>
#include <Core/src/log/ServerityLevelPolicy.hpp>
#include <Core/src/log/DefaultChannel.hpp>

#include <cstring>

using namespace gage;

#define dumb_logger log::EntryBuilder{ __FILE__, __FUNCTION__, __LINE__ }

class DumbChannel : public log::IChannel
{
public:
    void submit(log::Entry& e) override 
    {
        entry_ = e;
    }

    void attach_driver(std::shared_ptr<log::IDriver> driver) override {}
    void attach_policy(std::unique_ptr<log::IPolicy> policy) override {}

    log::Entry entry_;
};

class DumbDriver : public log::IDriver
{
public:
    void submit(log::Entry& e) override 
    {
        entry_ = e;
    }

    log::Entry entry_;
};

TEST_CASE("log-entry")
{
    DumbChannel chanl;
    dumb_logger.info("Hi").channel(&chanl);
    CHECK(std::strcmp("Hi", chanl.entry_.note_.c_str()) == 0);
    CHECK(chanl.entry_.level_ == log::Level::Info);
}


TEST_CASE("log-driver")
{
    log::DefaultChannel channel;
    auto driver1 = std::make_shared<DumbDriver>();
    auto driver2 = std::make_shared<DumbDriver>();

    channel.attach_driver(driver1);
    channel.attach_driver(driver2);
    dumb_logger.info("Hi").channel(&channel);

    CHECK(std::strcmp("Hi", driver1->entry_.note_.c_str()) == 0);
    CHECK(driver1->entry_.level_ == log::Level::Info);

    CHECK(std::strcmp("Hi", driver2->entry_.note_.c_str()) == 0);
    CHECK(driver2->entry_.level_ == log::Level::Info);
}

TEST_CASE("log-policy")
{
    log::DefaultChannel channel;
    auto driver1 = std::make_shared<DumbDriver>();
    
    auto policy = std::make_unique<log::ServerityLevelPolicy>(log::Level::Info);

    channel.attach_driver(driver1);
    channel.attach_policy(std::move(policy));
    dumb_logger.info("Hi").channel(&channel);


    CHECK(std::strcmp("Hi", driver1->entry_.note_.c_str()) == 0);
    CHECK(driver1->entry_.level_ == log::Level::Info);

    dumb_logger.debug("ABCD").channel(&channel);

    CHECK(std::strcmp("Hi", driver1->entry_.note_.c_str()) == 0);
    CHECK(driver1->entry_.level_ == log::Level::Info);

}