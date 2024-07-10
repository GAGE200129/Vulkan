#pragma once

#include "IChannel.hpp"


namespace gage::log
{
    class DefaultChannel : public IChannel
    {
    public:
        DefaultChannel(std::vector<std::shared_ptr<IDriver>> = {}, std::vector<std::shared_ptr<IPolicy>> policies = {});
        ~DefaultChannel();
        void submit(Entry& ) override;
        void attach_driver(std::shared_ptr<IDriver>) override;
        void attach_policy(std::shared_ptr<IPolicy>) override;
    private:
        std::vector<std::shared_ptr<IDriver>> driver_ptrs;
        std::vector<std::shared_ptr<IPolicy>> policy_ptrs;
    };
}