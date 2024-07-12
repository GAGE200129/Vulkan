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
    //Perspective divide
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords.x += 1.0; 
    proj_coords.y += 1.0; 
    proj_coords.xy *= 0.5;
    
    //proj_coords.y = -proj_coords.y;
    float closest_depth = texture(light_depth_map, proj_coords.st).r;
    float current_depth = proj_coords.z;

    vec3 ambient = vec3(0.3) * albedo * ao;

    if(current_depth > closest_depth)
        return vec4(ambient, 1.0);

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
    

    return vec4(Lo + ambient, 1);
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