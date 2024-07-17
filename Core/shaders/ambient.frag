#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"

layout (set = 1, binding = 0) uniform sampler2D g_buffer_albedo;


layout (location = 0) in vec2 fs_uv;
layout (location = 0) out vec4 out_color;



void main() 
{
	vec4 albedo = texture(g_buffer_albedo, fs_uv);
    out_color = vec4(albedo.rgb * ubo.ambient_light_color * ubo.ambient_light_intensity, 1); 
}