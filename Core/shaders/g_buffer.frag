#version 460
#extension GL_ARB_shading_language_include : require

#include "global_uniform_buffer.inc"
#include "material.inc"

layout(location = 0) in VSOutput
{
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
} fs_in; 
  


//output write
layout (location = 0) out vec3 out_g_buffer_position;
layout (location = 1) out vec3 out_g_buffer_normal;
layout (location = 2) out vec2 out_g_buffer_texcoord;


void main()
{
    out_g_buffer_position = fs_in.world_pos;
    out_g_buffer_normal = fs_in.normal;
    out_g_buffer_texcoord = fs_in.uv;
}