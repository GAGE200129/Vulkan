#define CASCADE_COUNT 3

#define SKY_STENCIL_ID 0
#define PBR_STENCIL_ID 1
#define TERRAIN_STENCIL_ID 2

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4x4 projection;
    mat4x4 inv_projection;
    mat4x4 view;
    mat4x4 inv_view;
    vec3 camera_position;

    vec3 ambient_light_color;
    float ambient_light_intensity;

    vec3 directional_light_direction; 
    vec3 directional_light_color;
    mat4x4 directional_light_proj_views[CASCADE_COUNT];
    float  directional_light_cascade_planes[CASCADE_COUNT];
    
} descriptor_set_0_ubo;

layout(set = 0, binding = 1) uniform sampler2DArray descriptor_set_0_directional_light_map;
