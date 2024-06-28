#version 450

layout(location = 0) in vec3 fs_normal;
layout(location = 1) in vec2 fs_uvs;

//output write
layout (location = 0) out vec4 outFragColor;

layout(binding = 1) uniform sampler2D u_texture;

layout(binding = 0) uniform UniformBuffer
{
    mat4x4 projection;
    mat4x4 view;
    vec3 light_position;
} ubo;

void main() 
{
	//outFragColor = vec4(fs_normal, 1.0);
	outFragColor = texture(u_texture, fs_uvs);
}