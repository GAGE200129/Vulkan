#version 460 core

//layout (location = 0) out vec3 out_g_buffer_position;
layout (location = 0) out vec3 out_g_buffer_normal;
layout (location = 1) out vec3 out_g_buffer_albedo;
layout (location = 2) out vec3 out_g_buffer_metalic_roughness;

layout(location = 0) in VSOutput
{
    //vec3 world_pos;
    vec2 tex_coord;
    vec3 normal;
} vs_in;

layout(set = 1, binding = 0) uniform sampler2D image;


void main()
{
    
    //out_g_buffer_position = vs_in.world_pos;
    out_g_buffer_normal = vs_in.normal;
    out_g_buffer_albedo = texture(image, vs_in.tex_coord).rgb;
    out_g_buffer_metalic_roughness = vec3(1, 1, 1);
}