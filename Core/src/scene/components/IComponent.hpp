#pragma once

namespace gage::gfx
{
    class Graphics;
}

namespace gage::scene::components
{
    class IComponent
    {
    public:
        virtual ~IComponent() = default;

        virtual void init() = 0;
        virtual void update(float delta) = 0;
        virtual void render(gfx::Graphics& gfx) = 0;
        virtual void shutdown() = 0;
    };
}