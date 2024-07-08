#version 460
#extension GL_ARB_shading_language_include : require

#include "global_uniform_buffer.inc"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uvs;
layout(location = 3) in vec4 in_tangent;


layout(location = 0) out vec3 fs_normal;
layout(location = 1) out vec2 fs_uvs;
layout(location = 2) out vec3 fs_world_pos;
layout(location = 3) out mat3 fs_TBN;


layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
};

void main() 
{   
    vec3 bitangent = cross(in_normal, in_tangent.xyz) * in_tangent.w;
    vec4 p = model_transform * vec4(in_pos, 1.0f);
	gl_Position = ubo.projection * ubo.view * p;
	fs_normal = normalize(mat3(transpose(inverse(model_transform))) * in_normal);
	fs_uvs = in_uvs;
    fs_world_pos = p.xyz;
    const vec3 T = normalize(vec3(model_transform * vec4(in_tangent.xyz, 0.0)));
    const vec3 B = normalize(vec3(model_transform * vec4(bitangent,      0.0)));
    const vec3 N = normalize(vec3(model_transform * vec4(in_normal,      0.0)));
    fs_TBN = mat3(T, B, N);
}