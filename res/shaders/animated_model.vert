#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUv;
layout(location = 3) in uvec4 inBoneIDs;
layout(location = 4) in vec4 inBoneWeights;


layout(location = 0) out vec2 FSUv;
layout(location = 1) flat out uvec4 FSBoneIDs;
layout(location = 2) out vec4 FSBoneWeights;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
  mat4 view;
  mat4 proj;
} ubo;

layout(set = 2, binding = 0) uniform BoneTransforms
{
  mat4 mat[100];
} boneTransforms;


layout( push_constant ) uniform Constants
{
  mat4 model;
} constants;


void main() {

  mat4 boneTransform = boneTransforms.mat[inBoneIDs[0]] * inBoneWeights[0];
  boneTransform += boneTransforms.mat[inBoneIDs[1]] * inBoneWeights[1];
  boneTransform += boneTransforms.mat[inBoneIDs[2]] * inBoneWeights[2];
  boneTransform += boneTransforms.mat[inBoneIDs[3]] * inBoneWeights[3];

  gl_Position = ubo.proj * ubo.view * constants.model * boneTransform * vec4(inPos, 1.0);
  FSUv = inUv;
  FSBoneIDs = inBoneIDs;
  FSBoneWeights = inBoneWeights;
}