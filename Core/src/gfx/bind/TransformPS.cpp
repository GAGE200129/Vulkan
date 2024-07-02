#include "TransformPS.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "../draw/Drawable.hpp"
#include <iostream>

namespace gage::gfx::bind
{
    TransformPS::TransformPS(Graphics &gfx, VkPipelineLayout layout,const draw::Drawable &parent) : 
        IBindable(gfx),
        parent(parent),
        layout(layout)
    {
        
    }
    void TransformPS::bind(Graphics &gfx)
    {
        vkCmdPushConstants(get_cmd(gfx), layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), glm::value_ptr(parent.get_world_transform()));
    }
}