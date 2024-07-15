#version 460
#extension GL_ARB_shading_language_include : require

#include "global_uniform_buffer.inc"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uvs;

layout(location = 0) out VSOutput
{
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
} vs_out;
 

layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
};

void main() 
{   
    vec4 p = model_transform * vec4(in_pos, 1.0f);
    vec4 p_view = ubo.view * vec4(p.xyz, 1.0);
	gl_Position = ubo.projection * p_view;
	vs_out.normal = mat3(transpose(inverse(model_transform))) * in_normal;
	vs_out.uv = in_uvs;
    vs_out.world_pos = p.xyz;
}