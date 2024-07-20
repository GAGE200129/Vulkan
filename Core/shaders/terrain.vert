#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"


layout(location = 0) in vec3 in_pos;

layout(location = 0) out VSOutput
{
    vec3 world_pos;
} vs_out;


void main()
{
    gl_PointSize = 1.0;
    gl_Position = ubo.projection * ubo.view * vec4(in_pos, 1.0);
    vs_out.world_pos = in_pos;
}