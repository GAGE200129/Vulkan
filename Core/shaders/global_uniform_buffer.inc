#include "directional_light.inc"

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4x4 projection;
    mat4x4 view;
    vec3 camera_position; float _padding;
    DirectionalLight directional_light;
    
} ubo;