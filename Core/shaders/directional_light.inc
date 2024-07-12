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
    in vec4 frag_pos_light_space,
    in sampler2D light_depth_map,
    in vec3 n,
    in vec3 to_cam_dir,
    in vec3 albedo,
    in float metalic,
    in float roughness,
    in float ao)
{
    //Shadow mapping

    const float bias = 0.005;
    //Perspective divide
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords.x += 1.0; 
    proj_coords.y += 1.0; 
    proj_coords.xy *= 0.5;
    
    //proj_coords.y = -proj_coords.y;
    float current_depth = proj_coords.z;

    vec3 ambient = vec3(0.3) * albedo * ao;

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(light_depth_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcf_depth = texture(light_depth_map, proj_coords.xy + vec2(x, y) * texel_size).r; 
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;


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
    

    return vec4(Lo * (1.0 - shadow) + ambient, 1);
}





#endif