#version 450

layout(location = 0) in vec3 inPos;


layout(set = 0, binding = 0) uniform GlobalUBO
{
  mat4 view;
  mat4 proj;
} globalUBO;


void main() { 
  gl_Position = globalUBO.proj * globalUBO.view * vec4(inPos, 1.0);
}