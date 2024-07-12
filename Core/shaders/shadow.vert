#version 460 core
#extension GL_ARB_shading_language_include : require

#include "global_uniform_buffer.inc"

layout(location = 0) in vec3 in_pos;

layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
};


void main()
{
    gl_Position = ubo.directional_light_proj_view * model_transform * vec4(in_pos, 1.0);
}