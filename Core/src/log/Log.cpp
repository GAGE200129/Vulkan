#include "Log.hpp"

#include "ioc/Container.hpp"

#include "DefaultChannel.hpp"
#include "DefaultFileDriver.hpp"
#include "StdoutDriver.hpp"
#include "ServerityLevelPolicy.hpp"

namespace gage::log
{
    IChannel* get_default_channel() 
    {
        static std::shared_ptr<IChannel> cache = ioc::Container::get().resolve<IChannel>();
        return cache.get();
    }

    void init() 
    {
        ioc::Container::get().register_factory<ServerityLevelPolicy>([]() {
            return std::make_shared<ServerityLevelPolicy>(Level::Error);
        });

        ioc::Container::get().register_factory<IChannel>([]() {
            std::vector<std::shared_ptr<IDriver>> drivers = {
                std::make_shared<StdoutDriver>(),
                std::make_shared<DefaultFileDriver>("log/default.log"),
            };

            auto result = std::make_shared<DefaultChannel>(drivers);
            result->attach_policy(ioc::Container::get().resolve<ServerityLevelPolicy>());
            return result;
        });
    }
}