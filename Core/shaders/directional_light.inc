#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT

#include "pbr_functions.inc"
#include "material.inc"

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
};

vec3 calculate_directional_light_phong(in DirectionalLight light, in vec3 n, in vec3 to_cam_dir, in vec3 albedo)
{
    //Diffuse light
    float diffuse_factor = max(dot(n, -light.direction), 0.0);
    vec3 diffuse_result;
    if(diffuse_factor > 0)
    {
        diffuse_result = light.color * 1.0 /*Light intensity*/ * diffuse_factor * material.color.rgb;
    }
    //Specular

    

    return diffuse_result * albedo;
}

vec4 calculate_directional_light_pbr(in DirectionalLight light,
    in vec3 frag_pos_world_space,
    in vec3 frag_pos_view_space,
    in sampler2DArray light_depth_map,
    in float cascade_planes[CASCADE_COUNT],
    in mat4x4 light_proj_views[CASCADE_COUNT],
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
        if (depth_value < cascade_planes[i])
        {
            layer = i;
            break;
        }
    }
    vec4 frag_pos_light_space = light_proj_views[layer] * vec4(frag_pos_world_space, 1.0);

    //Shadow mapping
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords.xy = (proj_coords.xy + 1.0) * 0.5;

    float current_depth = proj_coords.z;
    float sampled_depth = texture(light_depth_map, vec3(proj_coords.xy, layer)).r; 
    
    float bias = max(0.01 * (1.0 - dot(n_unsampled, -light.direction)), 0.001);
    //bias *= 1.0 / (ubo.directional_light_cascade_planes[layer] * 0.5f);


    
    float shadow = 1.0;
    if(sampled_depth + bias < current_depth)
    {
        shadow = 0;
    }

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




#endif