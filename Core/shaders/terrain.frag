#version 460 core

layout (location = 0) out vec3 out_g_buffer_position;
layout (location = 1) out vec3 out_g_buffer_normal;
layout (location = 2) out vec3 out_g_buffer_albedo;
layout (location = 3) out vec3 out_g_buffer_metalic_roughness;

layout(location = 0) in VSOutput
{
    vec3 world_pos;
} vs_in;


void main()
{
    
    out_g_buffer_position = vs_in.world_pos;
    out_g_buffer_normal = vec3(0, 1, 0);
    out_g_buffer_albedo = vec3(1, 0, 0);
    out_g_buffer_metalic_roughness = vec3(1, 1, 1);
}