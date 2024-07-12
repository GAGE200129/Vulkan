#version 460
#extension GL_ARB_shading_language_include : require

#include "global_uniform_buffer.inc"
#include "material.inc"

layout(location = 0) in FSOutput
{
    vec3 normal;
    vec2 uv;
    vec3 world_pos;
    vec4 world_pos_directional_light_space;
    mat3 TBN;
} fs_in;
 


//output write
layout (location = 0) out vec4 outFragColor;


void main() 
{
    vec3 to_cam_dir = normalize(ubo.camera_position - fs_in.world_pos);

    vec3 albedo = material.color.rgb;
    if(material.has_albedo)
        albedo *= texture(textures[0], fs_in.uv).rgb;
     
    float metalic = 1.0;
    float roughness = 0.0;
    if(material.has_metalic_roughness)
    {
        vec3 metalic_roughness = texture(textures[1], fs_in.uv).rgb;

        roughness = metalic_roughness.g;
        metalic = metalic_roughness.b;
    }

    vec3 n = normalize(fs_in.normal);
    if(material.has_normal)
    {
       n = texture(textures[2], fs_in.uv).rgb; 
       n = n * 2.0 - 1.0;
       n = normalize(fs_in.TBN * n); 
    }


    //outFragColor = texture(directional_light_map, fs_in.uv);
    //outFragColor = vec4(n, 1);
	outFragColor = calculate_directional_light_pbr(ubo.directional_light, fs_in.world_pos_directional_light_space, directional_light_map, n, to_cam_dir, albedo, metalic, roughness, 1.0);
    //outFragColor = vec4(calculate_directional_light_phong(ubo.directional_light, n, to_cam_dir, albedo), 1.0);
}