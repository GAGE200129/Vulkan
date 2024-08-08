#pragma once

#include "IComponent.hpp"


namespace gage::scene::systems
{
    class Scriptor;
}

namespace gage::hid
{
    class Keyboard;
    class Mouse;
}

namespace gage::scene::components
{
    class Script : public IComponent
    {
        friend class systems::Scriptor;
    public:
        Script(SceneGraph& scene, Node& node) :
            IComponent(scene, node) {}
        
        virtual ~Script() = default;

        inline virtual void init() {}
        inline virtual void update(float, const hid::Keyboard&, const hid::Mouse&)  {}
        inline virtual void late_update(float, const hid::Keyboard&, const hid::Mouse&)  {}
        inline virtual void shutdown() {}
        inline nlohmann::json to_json() const override { return { {"type", get_name()} }; };

        inline virtual void render_imgui()  {};
        inline const char* get_name() const final { return "Script"; };
    };
}