#version 460
#extension GL_ARB_shading_language_include : require

#include "global_uniform_buffer.inc"
#include "material.inc"

layout(location = 0) in FSOutput
{
    vec3 normal;
    vec2 uv;
    vec3 world_pos;
    mat3 TBN;
} fs_in; 
  


//output write
layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec3 out_g_buffer_position;




void main() 
{
    vec3 to_cam_dir = normalize(ubo.camera_position - fs_in.world_pos);

    vec3 albedo = material.color.rgb;
    if(material.has_albedo)
        albedo *= texture(textures[0], fs_in.uv).rgb;
     
    float metalic = 0.0;
    float roughness = 1.0;
    if(material.has_metalic_roughness)
    {
        vec3 metalic_roughness = texture(textures[1], fs_in.uv).rgb;

        roughness = metalic_roughness.g;
        metalic = metalic_roughness.b;
    }

    vec3 n_unsampled = normalize(fs_in.normal);
    vec3 n = n_unsampled;
    if(material.has_normal)
    {
       n = texture(textures[2], fs_in.uv).rgb; 
       n = n * 2.0 - 1.0;
       n = normalize(fs_in.TBN * n); 
    }

	outFragColor = calculate_directional_light_pbr(ubo.directional_light,
        fs_in.world_pos,
        (ubo.view * vec4(fs_in.world_pos, 1.0)).xyz,
        directional_light_map,
        ubo.directional_light_cascade_planes,
        ubo.directional_light_proj_views,
        n,
        n_unsampled,
        to_cam_dir,
        albedo, metalic, roughness, 1.0);

    out_g_buffer_position = fs_in.world_pos;
}