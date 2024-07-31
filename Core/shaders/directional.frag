#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/descriptor_set_0.inc"
#include "includes/directional_descriptor_set_1.inc"
#include "includes/pbr_functions.inc"




layout (location = 0) in vec2 fs_uv;
layout (location = 1) in vec2 fs_uv_non_scaled;

layout (location = 0) out vec4 out_color;


void main() 
{
	float depth = texture(g_buffers[0], fs_uv).r;
    vec4 clip_space_position = vec4(fs_uv_non_scaled * 2.0 - 1.0, depth, 1.0);
    vec4 view_space_position = descriptor_set_0_ubo.inv_projection * clip_space_position;
    // Perspective division
    view_space_position /= view_space_position.w;
    vec3 frag_pos_world_space = (descriptor_set_0_ubo.inv_view * view_space_position).xyz;


    vec3 n = texture(g_buffers[1], fs_uv).xyz;
    vec3 albedo = texture(g_buffers[2], fs_uv).rgb;

    vec3 metalic_roughness = texture(g_buffers[3], fs_uv).rgb;
    float metalic = metalic_roughness.b;
    float roughness = metalic_roughness.g;
    float ao = 1.0;


    vec3 frag_pos_view_space = (descriptor_set_0_ubo.view * vec4(frag_pos_world_space, 1.0)).xyz;
    vec3 to_cam_dir = normalize(descriptor_set_0_ubo.camera_position - frag_pos_world_space);


    float depth_value = abs(frag_pos_view_space.z);
    int layer = CASCADE_COUNT - 1;
    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        if (depth_value < descriptor_set_0_ubo.directional_light_cascade_planes[i])
        {
            layer = i;
            break;
        }
    }



    vec4 frag_pos_light_space = descriptor_set_0_ubo.directional_light_proj_views[layer] * vec4(frag_pos_world_space, 1.0);

    //Shadow mapping
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords.xy = (proj_coords.xy + 1.0) * 0.5;

    float current_depth = proj_coords.z;
    //float sampled_depth = texture(directional_light_map, vec3(proj_coords.xy, layer)).r; 
    
    float bias = max(0.05 * (1.0 - dot(n, -descriptor_set_0_ubo.directional_light_direction)), 0.005);
    bias *= 1.0 / (descriptor_set_0_ubo.directional_light_cascade_planes[layer] * 0.5f);

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(descriptor_set_0_directional_light_map, 0).xy;
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcf_depth = texture(descriptor_set_0_directional_light_map, vec3(proj_coords.xy + vec2(x, y) * texel_size, layer)).r; 
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    shadow = 1.0 - shadow;
    
    // float shadow = 0.0;
    // if(sampled_depth + bias > current_depth)
    // {
    //     shadow = 1.0;
    // }

    //Main pbr 
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; i++) 
    { 
        vec3 L = normalize(-descriptor_set_0_ubo.directional_light_direction);
        vec3 H = normalize(to_cam_dir + L);
    
        vec3 radiance     = descriptor_set_0_ubo.directional_light_color;
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

    out_color = vec4(Lo * shadow, 1);
}