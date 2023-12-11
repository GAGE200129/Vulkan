#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUv;


layout(location = 0) out vec2 FSUv;

layout(set = 0, binding = 0) uniform GlobalUBO
{
  mat4 view;
  mat4 proj;
} globalUBO;

layout(push_constant) uniform Constants
{
  mat4 model;
} constants;


void main() {
  gl_Position = globalUBO.proj * globalUBO.view * constants.model * vec4(inPos, 1.0);
  FSUv = inUv;
}