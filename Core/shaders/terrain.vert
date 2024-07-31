#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/descriptor_set_0.inc"


layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coord;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out VSOutput
{
    vec2 tex_coord;
    vec3 normal;
    float color;
} vs_out;

layout(set = 1, binding = 0) uniform TerrainData
{
    float min_height;
    float max_height;
    float uv_scale;
} terrain_data;


void main()
{
    
    gl_Position = descriptor_set_0_ubo.projection * descriptor_set_0_ubo.view * vec4(in_pos, 1.0);
    vs_out.tex_coord = in_tex_coord * terrain_data.uv_scale;
    vs_out.normal = in_normal;

    float delta = terrain_data.max_height - terrain_data.min_height;
    float height_ratio = (in_pos.y - terrain_data.min_height) / delta;
    vs_out.color = height_ratio;

}