#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"

layout (set = 1, binding = 0) uniform sampler2D g_buffers[];


layout (location = 0) in vec2 fs_uv;
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
	vec3 frag_pos_world_space = texture(g_buffers[0], fs_uv).xyz;
    vec3 n = texture(g_buffers[1], fs_uv).xyz;
    vec3 albedo = texture(g_buffers[2], fs_uv).rgb;

    // vec3 metalic_roughness = texture(g_buffers[3], fs_uv).rgb;
    // float metalic = metalic_roughness.b;
    // float roughness = metalic_roughness.g;
    // float ao = 1.0;

    vec3 to_cam_dir = normalize(ubo.camera_position - frag_pos_world_space);
    vec3 to_light_vec = position - frag_pos_world_space;
    float to_light_distance = length(to_light_vec);
    vec3 to_light_dir = normalize(to_light_vec);

    float attenuation = 1.0 / (constant + linear * to_light_distance + 
    		    exponent * (to_light_distance * to_light_distance));  

    //Specular

    vec3 reflect_dir = reflect(-to_cam_dir, n);
    float spec = pow(max(dot(to_cam_dir, reflect_dir), 0.0), 32);
    vec3 specular = 1.0f * spec * color * attenuation;  

    vec3 diffuse = albedo * max(dot(n, to_light_dir), 0.0) * intensity * color * attenuation;

    out_color = vec4(diffuse + specular, 1);
}