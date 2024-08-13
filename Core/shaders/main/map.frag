#version 460
#extension GL_ARB_shading_language_include : require

#include "../includes/descriptor_set_0.inc"

layout(location = 0) in VSOutput
{
    vec3 world_pos;
    vec3 normal;
} fs_in; 
  


//main pass write
layout (location = 0) out vec3 out_g_buffer_normal;
layout (location = 1) out vec3 out_g_buffer_albedo;
layout (location = 2) out vec3 out_g_buffer_metalic_roughness;



void main()
{
    out_g_buffer_normal = normalize(fs_in.normal);
    out_g_buffer_albedo = vec3(1, 0, 0);
    out_g_buffer_metalic_roughness = vec3(1, 1, 1);
}