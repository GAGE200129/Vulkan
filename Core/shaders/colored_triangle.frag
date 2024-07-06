#version 450
#extension GL_ARB_shading_language_include : require

#include "directional_light.inc"

layout(location = 0) in vec3 fs_normal;
layout(location = 1) in vec2 fs_uvs;
layout(location = 2) in vec3 fs_world_pos;

//output write
layout (location = 0) out vec4 outFragColor;

//Descriptor
layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4x4 projection;
    mat4x4 view;
    vec3 camera_position; float _padding;
    DirectionalLight directional_light;
} ubo;

layout(set = 1, binding = 0) uniform Material
{
    vec4 color;
    float specular_intensity;
    float specular_power;
    bool has_albedo;
    bool has_metalic_roughness;
} material;

layout(set = 1, binding = 1) uniform sampler2D textures[2];

void main() 
{
	const vec3 light_dir = normalize(-ubo.directional_light.direction);
    float diff = max(dot(fs_normal, light_dir), 0.0);
    vec3 diffuse = diff * ubo.directional_light.color;
    outFragColor = vec4(diffuse, 1.0);
}