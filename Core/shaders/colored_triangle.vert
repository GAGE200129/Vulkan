#version 460

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uvs;


layout(location = 0) out vec3 fs_normal;
layout(location = 1) out vec2 fs_uvs;
layout(location = 2) out vec3 fs_world_pos;


layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4x4 projection;
    mat4x4 view;
    vec3 point_light_position; float _padding;
    vec4 ambient;
    vec4 diffuse_color;
    float diffuse_intensity;
    float att_constant;
    float att_linear;
    float att_exponent;
} ubo;

layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
};

void main() 
{   
    vec4 p = model_transform * vec4(in_pos, 1.0f);
	gl_Position = ubo.projection * ubo.view * p;
	fs_normal = normalize(mat3(transpose(inverse(model_transform))) * in_normal);
	fs_uvs = in_uvs;
    fs_world_pos = p.xyz;
}