#ifndef GLOBAL_UNIFORM_BUFFER
#define GLOBAL_UNIFORM_BUFFER

#define CASCADE_COUNT 3

#include "directional_light.inc"


layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4x4 projection;
    mat4x4 view;
    vec3 camera_position;
    DirectionalLight directional_light;
    mat4x4 directional_light_proj_views[CASCADE_COUNT];
    float  directional_light_cascade_planes[CASCADE_COUNT];
    
} ubo;

layout(set = 0, binding = 1) uniform sampler2DArray directional_light_map;

#endif