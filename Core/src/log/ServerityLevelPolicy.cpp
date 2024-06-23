#include "ServerityLevelPolicy.hpp"
#include "Entry.hpp"

namespace gage::log
{
    ServerityLevelPolicy::ServerityLevelPolicy(Level level) : level_(level)
    {
    }
    bool ServerityLevelPolicy::transform_filter(Entry &e)
    {
        return e.level_ <= level_;
    }
}