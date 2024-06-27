#version 450

layout(location = 0) in vec3 fs_normal;
layout(location = 1) in vec2 fs_uvs;
layout(location = 2) in vec3 fs_color;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D u_texture;

void main() 
{
	//return red
	outFragColor = texture(u_texture, fs_uvs);
}