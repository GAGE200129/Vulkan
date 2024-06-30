#pragma once


namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class IData
    {
    public:
        IData() = default;
        virtual void destroy(Graphics& gfx) = 0;
        virtual ~IData();
    };
}