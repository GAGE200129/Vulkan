#pragma once

#include "IChannel.hpp"

#include <vector>
#include <memory>

namespace gage::log
{
    class DefaultChannel : public IChannel
    {
    public:
        DefaultChannel(std::vector<std::shared_ptr<IDriver>> = {});
        ~DefaultChannel();
        void submit(Entry& ) override;
        void attach_driver(std::shared_ptr<IDriver>) override;
        void attach_policy(std::unique_ptr<IPolicy>) override;
    private:
        std::vector<std::shared_ptr<IDriver>> driver_ptrs;
        std::vector<std::unique_ptr<IPolicy>> policy_ptrs;
    };
}