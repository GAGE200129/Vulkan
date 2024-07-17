#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"

layout (set = 1, binding = 0) uniform sampler2D g_buffers[];


layout (location = 0) in vec2 fs_uv;
layout (location = 0) out vec4 out_color;

layout(push_constant, std140) uniform PushConstant {
    layout(offset = 16) vec3 color;
	float intensity;
};



void main() 
{
	vec4 albedo = texture(g_buffers[2], fs_uv);
    out_color = vec4(albedo.rgb * color * intensity, 1);
}