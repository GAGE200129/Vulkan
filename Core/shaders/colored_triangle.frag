#version 450

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
    vec3 point_light_position; float _padding2;
    vec4 ambient;
    vec4 diffuse_color;
    float diffuse_intensity;
    float att_constant;
    float att_linear;
    float att_exponent;
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
	const vec3 to_light_vec = ubo.point_light_position - fs_world_pos;
    const float dist_to_light = length(to_light_vec);
    const vec3 dir_light_vec = to_light_vec / dist_to_light;

    const float attenuation = 1.0 / (ubo.att_constant + ubo.att_linear * dist_to_light + dist_to_light * ubo.att_exponent * ubo.att_exponent);
    const vec4 diffuse = ubo.diffuse_color * ubo.diffuse_intensity * max(0.0, dot(dir_light_vec, fs_normal)) * attenuation;

    outFragColor = diffuse;
}