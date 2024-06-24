#include "Log.hpp"

#include "ioc/Container.hpp"

#include "DefaultChannel.hpp"
#include "DefaultFileDriver.hpp"
#include "DefaultPolicy.hpp"
#include "StdoutDriver.hpp"

namespace gage::log
{
    IChannel* get_singeton_channel() 
    {
        static std::shared_ptr<IChannel> cache = ioc::Container::get().resolve<IChannel>();
        return cache.get();
    }

    void init() 
    {
        ioc::Container::get().register_factory<IChannel>([]() {
            std::vector<std::shared_ptr<IDriver>> drivers = {
                std::make_shared<StdoutDriver>(),
                std::make_shared<DefaultFileDriver>("log/default.log"),
            };
            std::vector<std::shared_ptr<IPolicy>> policies = {
                std::make_shared<DefaultPolicy>(Level::Error)
            };

            auto result = std::make_shared<DefaultChannel>(drivers, policies);
            return result;
        });
    }
}