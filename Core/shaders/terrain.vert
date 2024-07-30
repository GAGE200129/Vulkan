#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"


layout(location = 0) in vec3 in_pos;

layout(location = 0) out VSOutput
{
    vec3 world_pos;
    float color;
} vs_out;

layout(set = 1, binding = 0) uniform TerrainData
{
    float min_height;
    float max_height;
} terrain_data;


void main()
{
    
    gl_Position = ubo.projection * ubo.view * vec4(in_pos, 1.0);
    vs_out.world_pos = in_pos;

    float delta = terrain_data.max_height - terrain_data.min_height;
    float height_ratio = (in_pos.y - terrain_data.min_height) / delta;
    vs_out.color = height_ratio;
}