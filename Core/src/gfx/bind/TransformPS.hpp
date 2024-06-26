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
        TransformPS(Graphics& gfx,const VkPipelineLayout& layout, const draw::Drawable& parent);
        void bind(Graphics& gfx) override;
        void destroy(Graphics&) override {};
    private:
        const draw::Drawable& parent;
        const VkPipelineLayout& layout;
    };
}