#version 460
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uvs;
layout(location = 3) in uvec4 in_bone_ids;
layout(location = 4) in vec4 in_weights;

layout(location = 0) out VSOutput
{
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
} vs_out;

layout(set = 2, binding = 0) uniform Animation
{
    mat4x4 bone_matrices[100];
    bool enabled;
} animation;

 

layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
};

void main() 
{   
    vec4 total_position = model_transform * vec4(in_pos, 1.0);
    vec3 total_normal = mat3(transpose(inverse(model_transform))) * in_normal;
    if(animation.enabled)
    {
        total_position = vec4(0, 0, 0, 0);
        total_normal = vec3(0, 0, 0);
        for(uint i = 0 ; i < 4 ; i++)
        {
            vec4 local_position = animation.bone_matrices[in_bone_ids[i]] * vec4(in_pos,1.0f);
            total_position += local_position * in_weights[i];
            vec3 local_normal = mat3(animation.bone_matrices[in_bone_ids[i]]) * in_normal;
            total_normal += local_normal * in_weights[i];
        }
    }

    vec4 p_view = ubo.view * vec4(total_position.xyz, 1.0);
	gl_Position = ubo.projection * p_view;
	vs_out.normal = total_normal;
	vs_out.uv = in_uvs;
    vs_out.world_pos = total_position.xyz;
}