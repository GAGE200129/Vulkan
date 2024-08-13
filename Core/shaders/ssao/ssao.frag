#version 460 core
#extension GL_ARB_shading_language_include : require

layout(constant_id = 0) const int KERNEL_SIZE = 64;

#include "../includes/descriptor_set_0.inc"

layout(location = 0) out float out_color;
  
layout (location = 0) in vec2 fs_uv;
layout (location = 1) in vec2 fs_uv_non_scaled;

layout (set = 1, binding = 0) uniform sampler2D g_buffers[];
layout (set = 1, binding = 1) uniform UniformBlock
{
    vec4 samples[KERNEL_SIZE];
} uniform_block;


layout(push_constant, std140) uniform PS 
{
    layout(offset = 16)
    float radius;
    float bias;
    vec2 noise_scale;
    float resolution_scale;
} ps;

vec3 get_world_pos_from_depth(mat4 inverse_proj, mat4 inverse_view, vec2 uv, vec2 uv_none_scaled)
{
    
	float depth = texture(g_buffers[0], uv).r;
    vec4 clip_space_position = vec4(uv_none_scaled * 2.0 - 1.0, depth, 1.0);
    vec4 view_space_position = inverse_proj * clip_space_position;
    // Perspective division
    view_space_position /= view_space_position.w;
    vec3 frag_pos_world_space = (inverse_view * view_space_position).xyz;

    return frag_pos_world_space;
}


void main()
{
    vec3 frag_pos_world_space = get_world_pos_from_depth(descriptor_set_0_ubo.inv_projection, descriptor_set_0_ubo.inv_view, fs_uv, fs_uv_non_scaled);
    vec3 n    = texture(g_buffers[1], fs_uv).rgb;

    vec3 frag_pos_view_space   = (descriptor_set_0_ubo.view * vec4(frag_pos_world_space, 1.0)).xyz;
    vec3 n_view_space = (descriptor_set_0_ubo.view * vec4(n, 0.0)).xyz;
    vec3 random_vec = texture(g_buffers[2], fs_uv * ps.noise_scale).rgb; 

    vec3 tangent   = normalize(random_vec - n_view_space * dot(random_vec, n_view_space));
    vec3 bitangent = cross(n_view_space, tangent);
    mat3 TBN       = mat3(tangent, bitangent, n_view_space);
    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        // get sample position
        vec3 sample_pos = TBN * uniform_block.samples[i].xyz; // from tangent to view-space
        sample_pos = frag_pos_view_space + sample_pos * ps.radius; 


        vec4 offset = vec4(sample_pos, 1.0);
        offset      = descriptor_set_0_ubo.projection * offset; // from view to clip-space
        offset.xyz /= offset.w;                // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5;  // transform to range 0.0 - 1.0  

        vec3 sample_pos_world_space = get_world_pos_from_depth(descriptor_set_0_ubo.inv_projection, descriptor_set_0_ubo.inv_view, offset.xy * ps.resolution_scale, fs_uv_non_scaled);
        vec3 sample_pos_view_space = (descriptor_set_0_ubo.view * vec4(sample_pos_world_space, 1.0)).xyz;
        float sample_depth = sample_pos_view_space.z; 
        float range_check = smoothstep(0.0, 1.0, ps.radius / abs(frag_pos_view_space.z - sample_depth)); //Range check not working
        occlusion += (sample_depth >= sample_pos.z + ps.bias ? 1.0 : 0.0) * range_check;

    }    

    occlusion = 1.0 - (occlusion / KERNEL_SIZE);
    out_color = occlusion;  

}