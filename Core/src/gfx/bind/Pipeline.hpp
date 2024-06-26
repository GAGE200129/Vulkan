#pragma once

#include "IBindable.hpp"

#include "../PipelineBuilder.hpp"

namespace gage::gfx::bind
{
    class Pipeline : public IBindable
    {
    public:
        Pipeline(Graphics& gfx, PipelineBuilder& builder);
        void bind(Graphics& gfx);
        void destroy(Graphics& gfx);

        const VkPipelineLayout& get_layout() const;
    private:
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
    };
}