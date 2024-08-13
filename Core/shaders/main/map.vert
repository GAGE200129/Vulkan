#version 460
#extension GL_ARB_shading_language_include : require

#include "../includes/descriptor_set_0.inc"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;


layout(location = 0) out VSOutput
{
    vec3 world_pos;
    vec3 normal;
} vs_out;


 

layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
}ps;

void main() 
{   
    vec4 p = ps.model_transform * vec4(in_pos, 1.0);
    vec4 p_view = descriptor_set_0_ubo.view * vec4(p.xyz, 1.0);
	gl_Position = descriptor_set_0_ubo.projection * p_view;
    vs_out.world_pos = p.xyz;
    vs_out.normal = mat3x3(ps.model_transform) * in_normal;
}