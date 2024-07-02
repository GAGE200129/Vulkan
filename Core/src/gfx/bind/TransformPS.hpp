#pragma once

#include "IBindable.hpp"

namespace gage::gfx::draw
{
    class Drawable;
}

namespace gage::gfx::bind
{
    class TransformPS : public IBindable
    {
    public:
        TransformPS(Graphics& gfx, VkPipelineLayout layout, const draw::Drawable& parent);
        virtual ~TransformPS() = default;
        void bind(Graphics& gfx) final;
    private:
        const draw::Drawable& parent;
        VkPipelineLayout layout;
    };
}