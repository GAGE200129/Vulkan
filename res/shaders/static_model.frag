#version 450

layout(location = 0) in vec2 FSUv;
layout(location = 1) in vec3 FSNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D dSampler;

const vec3 gLightDirection = vec3(-1, -1, -1);

void main() { 
    outColor = texture(dSampler, FSUv) * dot(-gLightDirection, normalize(FSNormal));
}