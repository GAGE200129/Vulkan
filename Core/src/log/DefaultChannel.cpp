#include <pch.hpp>
#include "DefaultChannel.hpp"

#include "IDriver.hpp"
#include "IPolicy.hpp"

namespace gage::log
{
    DefaultChannel::DefaultChannel(std::vector<std::shared_ptr<IDriver>> drivers, std::vector<std::shared_ptr<IPolicy>> policies) :
        driver_ptrs{ std::move(drivers) },
        policy_ptrs{ std::move(policies) }
    {
    }

    DefaultChannel::~DefaultChannel()
    {

    }

    void DefaultChannel::submit(Entry & e)
    {
        for(auto& policy : this->policy_ptrs) 
        {
            if(!policy->transform_filter(e))
            {
                return;
            }
        }
        for(auto& driver : this->driver_ptrs) 
        {
            driver->submit(e);
        }
    }
    void DefaultChannel::attach_driver(std::shared_ptr<IDriver> driver)
    {
        driver_ptrs.push_back(std::move(driver));
    }

    void DefaultChannel::attach_policy(std::shared_ptr<IPolicy> policy) 
    {
        policy_ptrs.push_back(std::move(policy));
    }
}