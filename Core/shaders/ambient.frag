#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/descriptor_set_0.inc"
#include "includes/ambient_descriptor_set_1.inc"



layout (location = 0) in vec2 fs_uv;
layout (location = 1) in vec2 fs_uv_non_scaled;

layout (location = 0) out vec4 out_color;



void main() 
{
    vec3 ambient_light = descriptor_set_0_ubo.ambient_light_color * descriptor_set_0_ubo.ambient_light_intensity;
    if(texture(g_buffer_stencil, fs_uv).r == 0)
    {
        out_color = vec4(ambient_light, 1);
    }
    else
    {
        vec4 albedo = texture(g_buffers[0], fs_uv);
        

        //Blur the kernel
        vec2 texel_size = 1.0 / vec2(textureSize(g_buffers[1], 0));
        float ssao_result = 0.0;
        for (int x = -2; x <= 2; ++x) 
        {
            for (int y = -2; y <= 2; ++y) 
            {
                vec2 offset = vec2(float(x), float(y)) * texel_size;
                ssao_result += texture(g_buffers[1], fs_uv + offset).r;
            }
        }
        ssao_result /= (16.0);

        //Fog
        float depth = texture(g_buffers[2], fs_uv).r;

        vec4 clip_space_position = vec4(fs_uv_non_scaled * 2.0 - 1.0, depth, 1.0);
        vec4 view_space_position = descriptor_set_0_ubo.inv_projection * clip_space_position;
        // Perspective division
        view_space_position /= view_space_position.w;
        vec3 frag_pos_world_space = (descriptor_set_0_ubo.inv_view * view_space_position).xyz;

        //Linear fog
        float fog_factor;
        {
            float distance = length(frag_pos_world_space - descriptor_set_0_ubo.camera_position);
            float range = descriptor_set_0_ubo.ambient_fog_end - descriptor_set_0_ubo.ambient_fog_begin;
            float fog_dist = descriptor_set_0_ubo.ambient_fog_end - distance;
            fog_factor = 1.0 - clamp(fog_dist / range, 0.0, 1.0);
        }
        
        vec3 final_color = albedo.rgb * ambient_light * ssao_result;
        final_color = mix(final_color, ambient_light, fog_factor);
        out_color = vec4(final_color, 1.0); 
    }
}