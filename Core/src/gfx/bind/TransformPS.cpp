#include "TransformPS.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "../draw/Drawable.hpp"
#include <iostream>

namespace gage::gfx::bind
{
    TransformPS::TransformPS(Graphics &, VkPipelineLayout layout,const draw::Drawable &parent) : 
        parent(parent),
        layout(layout)
    {
        
    }
    void TransformPS::bind(Graphics &gfx)
    {
        glm::mat4x4 mvp = gfx.get_projection() * parent.get_world_transform();
        vkCmdPushConstants(get_cmd(gfx), layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), glm::value_ptr(mvp));
    }
}