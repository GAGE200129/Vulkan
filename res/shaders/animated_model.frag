#version 450

layout(location = 0) in vec2 FSUv;
layout(location = 1) flat in uvec4 FSBoneIDs;
layout(location = 2) in vec4 FSBoneWeights;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D dSampler;

void main() { 
    outColor = texture(dSampler, FSUv);
}