#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) in VSOutput
{
    vec3 normal;
    vec2 uv;
    vec3 world_pos;
} vs_in[];

layout(location = 0) out FSOutput
{
    vec3 normal;
    vec2 uv;
    vec3 world_pos;
    mat3 TBN;
} fs_out;

layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
};

void main() {    
    //Generate tangent
    const vec3 edge0 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    const vec3 edge1 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    const vec2 delta_uv0 = vs_in[1].uv - vs_in[0].uv;
    const vec2 delta_uv1 = vs_in[2].uv - vs_in[0].uv;

    const float inv_det = 1.0 / (delta_uv0.x * delta_uv1.y - delta_uv1.x * delta_uv0.y);

    const vec3 tangent = vec3(inv_det * (delta_uv1.y * edge0 - delta_uv0.y * edge1));
    const vec3 bitangent = vec3(inv_det * (-delta_uv1.x * edge0 + delta_uv0.x * edge1));

    const vec3 T = normalize(vec3(model_transform * vec4(tangent, 0)));
    const vec3 B = normalize(vec3(model_transform * vec4(bitangent, 0)));
    

    for(int i = 0; i < 3; i++)
    {
        gl_Position = gl_in[i].gl_Position; 
        fs_out.normal = vs_in[i].normal;
        fs_out.uv = vs_in[i].uv;
        fs_out.world_pos = vs_in[i].world_pos;
        fs_out.TBN = mat3(T, B, normalize(vs_in[i].normal));
        EmitVertex();
    }
    
    EndPrimitive();
} 