#pragma once
#include "IBindable.hpp"

#include "Texture.hpp"

#include <vk_mem_alloc.h>

namespace gage::gfx::bind
{
    class Sampler : public IBindable
    {
    public:
        Sampler(Graphics& gfx);

        void bind(Graphics& gfx) override;
        void destroy(Graphics& gfx) override;

        VkSampler get_sampler() const;
    private:
        VkSampler sampler{};
    };
}