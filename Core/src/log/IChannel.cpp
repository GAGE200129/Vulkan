#include "IChannel.hpp"

#include "IDriver.hpp"

namespace gage::log
{
    Channel::Channel(std::vector<std::shared_ptr<IDriver>> drivers) :
        driver_ptrs{ std::move(drivers) }
    {
    }

    void Channel::submit(Entry & e)
    {
        for(auto& driver : this->driver_ptrs) 
        {
            driver->submit(e);
        }
    }
    void Channel::attach_driver(std::shared_ptr<IDriver> driver)
    {
        driver_ptrs.push_back(std::move(driver));
    }
}