#pragma once

#include "IPolicy.hpp"
#include "Level.hpp"

namespace gage::log
{
    class ServerityLevelPolicy : public IPolicy 
    {
    public:
        ServerityLevelPolicy(Level level);
        bool transform_filter(Entry& e) override;
    private:
        Level level_;
    };
}