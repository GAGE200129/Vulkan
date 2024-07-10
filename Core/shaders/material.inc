#ifndef MATERIAL
#define MATERIAL


layout(set = 1, binding = 0) uniform Material
{
    vec4 color;
    float specular_intensity;
    float specular_power;
    bool has_albedo;
    bool has_metalic_roughness;
    bool has_normal;
} material;
layout(set = 1, binding = 1) uniform sampler2D textures[3];

#endif