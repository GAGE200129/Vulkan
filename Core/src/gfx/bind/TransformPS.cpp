#include "TransformPS.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>


namespace gage::gfx::bind
{
    TransformPS::TransformPS(Graphics &, const VkPipelineLayout& layout,const draw::Drawable &parent) : 
        parent(parent),
        layout(layout)
    {
        
    }
    void TransformPS::bind(Graphics &gfx)
    {
        glm::mat4x4 mvp = glm::mat4x4(1.0f);
        vkCmdPushConstants(get_cmd(gfx), layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), glm::value_ptr(mvp));
    }
}