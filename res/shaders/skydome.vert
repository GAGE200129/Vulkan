#version 450

layout(location = 0) in vec3 inPos;

layout(location = 0) out float FSHeight;

layout(set = 0, binding = 0) uniform GlobalUBO
{
    mat4 view;
    mat4 proj;
} globalUBO;


void main() { 
    FSHeight = inPos.y;
    gl_Position = globalUBO.proj * mat4(mat3(globalUBO.view)) * vec4(inPos.x, inPos.y - 0.2, inPos.z, 1.0);
}