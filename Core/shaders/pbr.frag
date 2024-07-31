#version 460
#extension GL_ARB_shading_language_include : require

#include "includes/descriptor_set_0.inc"
#include "includes/pbr_descriptor_set_1_frag.inc"

layout(location = 0) in VSOutput
{
    vec3 world_pos;
    vec3 normal;
    vec2 uv;
    mat3 TBN;
} fs_in; 
  


//output write
//layout (location = 0) out vec3 out_g_buffer_position;
layout (location = 0) out vec3 out_g_buffer_normal;
layout (location = 1) out vec3 out_g_buffer_albedo;
layout (location = 2) out vec3 out_g_buffer_metalic_roughness;



void main()
{

    vec3 albedo = material.color.rgb;
    if(material.has_albedo)
        albedo *= texture(textures[0], fs_in.uv).rgb;
     
    vec3 metalic_roughness = vec3(0.0, 1.0, 0.0);
    if(material.has_metalic_roughness)
    {
        metalic_roughness = texture(textures[1], fs_in.uv).rgb;

    }

    vec3 n = normalize(fs_in.normal);
    if(material.has_normal)
    {
       n = texture(textures[2], fs_in.uv).rgb; 
       n = n * 2.0 - 1.0;
       n = normalize(fs_in.TBN * n); 
    }


    //out_g_buffer_position = fs_in.world_pos;
    out_g_buffer_normal = n;
    out_g_buffer_albedo = albedo;
    out_g_buffer_metalic_roughness = metalic_roughness;
}