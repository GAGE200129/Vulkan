#version 460

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uvs;


layout(location = 0) out vec3 fs_normal;
layout(location = 1) out vec2 fs_uvs;
layout(location = 2) out vec3 fs_color;

layout(push_constant, std140) uniform PushConstant {
    mat4x4 mvp;
};

const vec3 colors[] = 
{
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
	vec3(1, 1, 0),
	vec3(0, 1, 1),
	vec3(1, 0, 1),
};

void main() 
{
	//output the position of each vertex
	gl_Position = mvp * vec4(in_pos, 1.0f);
	fs_normal = in_normal;
	fs_uvs = in_uvs;
	fs_color = colors[gl_VertexIndex % 6];
}