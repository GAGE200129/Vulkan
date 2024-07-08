#version 460
#extension GL_ARB_shading_language_include : require

#include "global_uniform_buffer.inc"

layout(location = 0) in vec3 fs_normal;
layout(location = 1) in vec2 fs_uvs;
layout(location = 2) in vec3 fs_world_pos;
layout(location = 3) in mat3 fs_TBN;

//output write
layout (location = 0) out vec4 outFragColor;


layout(set = 1, binding = 0) uniform Material
{
    vec4 color;
    float specular_intensity;
    float specular_power;
    bool has_albedo;
    bool has_metalic_roughness;
    bool has_normal;
} material;

layout(set = 1, binding = 1) uniform sampler2D textures[3];

void main() 
{
    vec3 to_cam_dir = normalize(ubo.camera_position - fs_world_pos);

    vec3 albedo = material.color.rgb;
    if(material.has_albedo)
        albedo *= texture(textures[0], fs_uvs).rgb;
    
    float metalic = 1.0;
    float roughness = 0.0;
    if(material.has_metalic_roughness)
    {
        vec3 metalic_roughness = texture(textures[1], fs_uvs).rgb;

        roughness = metalic_roughness.g;
        metalic = metalic_roughness.b;
    }

    vec3 n = fs_normal;
    if(material.has_normal)
    {
        n = texture(textures[2], fs_uvs).rgb * 2.0 - 1.0;
        n = normalize(fs_TBN * n);
    }

	outFragColor = calculate_directional_light_pbr(ubo.directional_light, n, to_cam_dir, albedo, metalic, roughness, 1.0);
}