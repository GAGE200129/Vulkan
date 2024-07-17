#version 460
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"
#include "includes/material.inc"

layout(location = 0) in VSOutput
{
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
} fs_in; 
  


//output write
layout (location = 0) out vec3 out_g_buffer_position;
layout (location = 1) out vec3 out_g_buffer_normal;
layout (location = 2) out vec3 out_g_buffer_albedo;


void main()
{

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

    vec3 n = normalize(fs_in.normal);
    if(material.has_normal)
    {
       n = texture(textures[2], fs_in.uv).rgb; 
       n = n * 2.0 - 1.0;
       n = normalize(fs_in.TBN * n); 
    }


    out_g_buffer_position = fs_in.world_pos;
    out_g_buffer_normal = n;
    out_g_buffer_albedo = albedo;
}