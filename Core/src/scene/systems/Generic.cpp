#include <pch.hpp>
#include "Generic.hpp"

namespace gage::scene::systems
{
    Generic::Generic()
    {

    }
    Generic::~Generic()
    {

    }
    void Generic::init()
    {
        for(const auto& script : scripts)
        {
            script->init();
        }
    }
    void Generic::update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse)
    {
        for(const auto& script : scripts)
        {
            script->update(delta, keyboard, mouse);
        }
    }
    void Generic::late_update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse)
    {
        for(const auto& script : scripts)
        {
            script->late_update(delta, keyboard, mouse);
        }
    }
    void Generic::shutdown()
    {
        for(const auto& script : scripts)
        {
            script->shutdown();
        }
    }

    void Generic::add_script(std::unique_ptr<components::Script> script)
    {
        scripts.push_back(std::move(script));
    }
}