#version 450

layout(location = 0) in vec2 FSUv;
layout(location = 1) flat in uvec4 FSBoneIDs;
layout(location = 2) in vec4 FSBoneWeights;
layout(location = 3) in vec3 FSNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D dSampler;

const vec3 gLightDir = vec3(-1, -1, 0);

void main() {

    outColor = max(dot(FSNormal, normalize(-gLightDir)), 0.01) * texture(dSampler, FSUv);
}