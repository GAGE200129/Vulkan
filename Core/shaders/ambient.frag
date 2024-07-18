#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/global_uniform_buffer.inc"

layout (set = 1, binding = 0) uniform sampler2D g_buffers[];


layout (location = 0) in vec2 fs_uv;
layout (location = 0) out vec4 out_color;



void main() 
{
	vec4 albedo = texture(g_buffers[0], fs_uv);

    //Blur the kernel
    vec2 texel_size = 1.0 / vec2(textureSize(g_buffers[1], 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texel_size;
            result += texture(g_buffers[1], fs_uv + offset).r;
        }
    }
    out_color = vec4(albedo.rgb * ubo.ambient_light_color * ubo.ambient_light_intensity * result, 1); 
}