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
        
        VkDescriptorSetLayout get_desc_set_layout(std::string name);

        const VkPipelineLayout& get_layout() const;
    private:
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        std::unordered_map<std::string, VkDescriptorSetLayout> layouts{};
    };
}