#version 460 core
#extension GL_ARB_shading_language_include : require

#include "../includes/descriptor_set_0.inc"

layout (set = 1, binding = 0) uniform sampler2D g_buffers[];


layout (location = 0) in vec2 fs_uv;
layout (location = 1) in vec2 fs_uv_non_scaled;

layout (location = 0) out vec4 out_color;


layout(push_constant, std140) uniform PushConstant {
    layout(offset = 16)
    vec3 position; 
    float intensity;
    vec3 color;
    float constant;
    float linear;
    float exponent;
};

void main() 
{
    float depth = texture(g_buffers[0], fs_uv).r;

    //uint stencil = texture(g_buffers[0], fs_uv);
    vec4 clip_space_position = vec4(fs_uv_non_scaled * 2.0 - 1.0, depth, 1.0);
    vec4 view_space_position = descriptor_set_0_ubo.inv_projection * clip_space_position;
    // Perspective division
    view_space_position /= view_space_position.w;
    vec3 frag_pos_world_space = (descriptor_set_0_ubo.inv_view * view_space_position).xyz;

    vec3 n = texture(g_buffers[1], fs_uv).xyz;
    vec3 albedo = texture(g_buffers[2], fs_uv).rgb;

    vec3 to_cam_dir = normalize(descriptor_set_0_ubo.camera_position - frag_pos_world_space);
    vec3 to_light_vec = position - frag_pos_world_space;
    float to_light_distance = length(to_light_vec);
    vec3 to_light_dir = normalize(to_light_vec);

    float attenuation = 1.0 / (constant + linear * to_light_distance + 
    		    exponent * (to_light_distance * to_light_distance));  

    //Specular

    vec3 reflect_dir = reflect(-to_light_dir, n);
    float spec = pow(max(dot(to_cam_dir, reflect_dir), 0.0), 32);
    vec3 specular = 1.0f * spec * color * attenuation;  

    vec3 diffuse = albedo * max(dot(n, to_light_dir), 0.0) * intensity * color * attenuation;

    out_color = vec4(diffuse + specular, 1);
}