#version 460 core

layout (location = 0) out vec2 fs_uv;


layout(push_constant, std140) uniform PushConstant {
    float resolution_scale;
};

void main() 
{
	vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	fs_uv = uv * resolution_scale;
	gl_Position = vec4(uv * 2.0f - 1.0f, 0.0f, 1.0f);
}