#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec2 FSUv;

layout(set = 0, binding = 0) uniform GlobalUBO
{
    mat4 view;
    mat4 proj;
} globalUBO;


void main() { 
    FSUv = inUv;
    gl_Position = globalUBO.proj * globalUBO.view * vec4(inPos, 1.0);
}