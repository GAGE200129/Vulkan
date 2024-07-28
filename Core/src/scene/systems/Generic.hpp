#pragma once

#include "../components/Script.hpp"
#include <vector>
#include <memory>

namespace gage::hid
{
    class Keyboard;
    class Mouse;
}

namespace gage::scene::systems
{
    class Generic
    {
    public:
        Generic();
        ~Generic();
        
        void init();
        void update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse);
        void late_update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse);
        void shutdown();

        void add_script(std::unique_ptr<components::Script> script);
    private:
        std::vector<std::unique_ptr<components::Script>> scripts; 
    };
}