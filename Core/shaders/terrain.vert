#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/descriptor_set_0.inc"


layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coord;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out VSOutput
{
    vec2 tex_coord;
    vec3 normal;
} vs_out;



void main()
{
    
    gl_Position = descriptor_set_0_ubo.projection * descriptor_set_0_ubo.view * vec4(in_pos, 1.0);
    vs_out.tex_coord = in_tex_coord * 200.0;
    vs_out.normal = in_normal;


}