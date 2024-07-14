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

vec4 calculate_directional_light_pbr(in DirectionalLight light,
    in vec3 frag_pos_world_space,
    in vec3 frag_pos_view_space,
    in sampler2DArray light_depth_map,
    in vec3 n,
    in vec3 n_unsampled,
    in vec3 to_cam_dir,
    in vec3 albedo,
    in float metalic,
    in float roughness,
    in float ao)
{ 
    float depth_value = abs(frag_pos_view_space.z);
    int layer = CASCADE_COUNT - 1;
    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        if (depth_value < ubo.directional_light_cascade_planes[i])
        {
            layer = i;
            break;
        }
    }
    vec4 frag_pos_light_space = ubo.directional_light_proj_views[layer] * vec4(frag_pos_world_space, 1.0);

    //Shadow mapping
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords.xy = (proj_coords.xy + 1.0) * 0.5;

    float current_depth = proj_coords.z;
    
    float bias = max(0.05 * (1.0 - dot(n_unsampled, -light.direction)), 0.005);
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(light_depth_map, 0).xy;
    for (int x=-1 ; x <= 1 ; x++){
        for (int y= -1 ; y <= 1 ; y++){
            float sampled_depth = texture(light_depth_map, vec3(proj_coords.xy + vec2(x, y) * texel_size, layer)).r; 
            if ( sampled_depth + bias <  current_depth  ){
                shadow += 1.0;
            } 

        }
    }
    if(proj_coords.z > 1.0) 
        shadow = 0.0;
    shadow = 1.0 - (shadow / 9.0);
    

    //Main pbr
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; i++) 
    { 
        vec3 L = normalize(-light.direction);
        vec3 H = normalize(to_cam_dir + L);
    
        vec3 radiance     = light.color;
        vec3 F0 = vec3(0.04); 
        F0      = mix(F0, albedo, metalic);
        vec3 F  = fresnelSchlick(max(dot(H, to_cam_dir), 0.0), F0);
        float NDF = DistributionGGX(n, H, roughness);       
        float G   = GeometrySmith(n, to_cam_dir, L, roughness); 
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(n, to_cam_dir), 0.0) * max(dot(n, L), 0.0)  + 0.0001;
        vec3 specular     = numerator / denominator; 
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        
        kD *= 1.0 - metalic;
        float NdotL = max(dot(n, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    } 
    vec3 ambient = vec3(0.3) * albedo * ao;

    //return vec4(shadow, shadow, shadow, 1.0);
    return vec4(Lo * shadow + ambient, 1);
}



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
        n,
        n_unsampled,
        to_cam_dir,
        albedo, metalic, roughness, 1.0);
}