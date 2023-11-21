#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;


layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObject
{
  mat4 view;
  mat4 proj;
} ubo;

layout( push_constant ) uniform Constants
{
  mat4 model;
} constants;


void main() {
    gl_Position = ubo.proj * ubo.view * constants.model * vec4(inPos, 1.0);
    fragColor = inColor;
}