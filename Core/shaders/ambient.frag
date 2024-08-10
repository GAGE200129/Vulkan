#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/descriptor_set_0.inc"

layout (set = 1, binding = 0) uniform sampler2D g_buffers[];
layout (set = 1, binding = 1) uniform usampler2D g_buffer_stencil;

layout(push_constant) uniform PushConstant
{
    layout(offset = 16)
    float time;
    float fbm_scale;
    float fbm_factor;
    float height;
} ps;


layout (location = 0) in vec2 fs_uv;
layout (location = 1) in vec2 fs_uv_non_scaled;

layout (location = 0) out vec4 out_color;

void process_ambient_light()
{
    vec3 ambient_light = descriptor_set_0_ubo.ambient_light_color * descriptor_set_0_ubo.ambient_light_intensity;
    vec4 albedo = texture(g_buffers[0], fs_uv);
    

    //Blur the kernel
    vec2 texel_size = 1.0 / vec2(textureSize(g_buffers[1], 0));
    float ssao_result = 0.0;
    for (int x = -2; x <= 2; ++x) 
    {
        for (int y = -2; y <= 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texel_size;
            ssao_result += texture(g_buffers[1], fs_uv + offset).r;
        }
    }
    ssao_result /= (16.0);

    //Fog
    float depth = texture(g_buffers[2], fs_uv).r;

    vec4 clip_space_position = vec4(fs_uv_non_scaled * 2.0 - 1.0, depth, 1.0);
    vec4 view_space_position = descriptor_set_0_ubo.inv_projection * clip_space_position;
    // Perspective division
    view_space_position /= view_space_position.w;
    vec3 frag_pos_world_space = (descriptor_set_0_ubo.inv_view * view_space_position).xyz;

    //Linear fog
    float fog_factor;
    {
        float distance = length(frag_pos_world_space - descriptor_set_0_ubo.camera_position);
        float range = descriptor_set_0_ubo.ambient_fog_end - descriptor_set_0_ubo.ambient_fog_begin;
        float fog_dist = descriptor_set_0_ubo.ambient_fog_end - distance;
        fog_factor = 1.0 - clamp(fog_dist / range, 0.0, 1.0);
    }
    
    vec3 final_color = albedo.rgb * ambient_light * ssao_result;
    final_color = mix(final_color, ambient_light, fog_factor);
    out_color = vec4(final_color, 1.0); 
}


float sd_sphere(vec3 p, float radius)
{
    return length(p) - radius;
}

float sd_box(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0)) + min(0, max(q.x, max(q.y, q.z)));
}



float hash(float p) { p = fract(p * 0.011); p *= p + 7.5; p *= p + p; return fract(p); }
//float hash(vec2 p) {vec3 p3 = fract(vec3(p.xyx) * 0.13); p3 += dot(p3, p3.yzx + 3.333); return fract((p3.x + p3.y) * p3.z); }


float noise(vec3 x) {
    const vec3 step = vec3(110, 241, 171);

    vec3 i = floor(x);
    vec3 f = fract(x);
 
    // For performance, compute the base input to a 1D hash from the integer part of the argument and the 
    // incremental change to the 1D based on the 3D -> 1D wrapping
    float n = dot(i, step);

    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix( hash(n + dot(step, vec3(0, 0, 0))), hash(n + dot(step, vec3(1, 0, 0))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 0))), hash(n + dot(step, vec3(1, 1, 0))), u.x), u.y),
               mix(mix( hash(n + dot(step, vec3(0, 0, 1))), hash(n + dot(step, vec3(1, 0, 1))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 1))), hash(n + dot(step, vec3(1, 1, 1))), u.x), u.y), u.z);
}

// float sample_noise_image(vec3 x) {
//     vec3 p = floor(x);
//     vec3 f = fract(x);
//     vec3 u = f * f * (3.0 - 2.0 * f);

//     vec2 uv = (p.xy + vec2(37.0, 239.0) * p.z) + u.xy;
//     vec2 tex = textureLod(noise_image, (uv + 0.5) / 256.0, 0.0).xx;

//     return mix( tex.x, tex.y, u.z ) * 2.0 - 1.0;
// }

// float sample_noise_image(vec3 p)
// {
//     vec3 f = fract(p);
//     vec3 u = f * f * (3. - 2. * f);
//     vec2 uv = (p.xy + vec2(37.0, 239.0) * p.z) + u.xy;
//     vec2 tex = textureLod(noise_image,(uv + 0.5) / 256.0, 0.0).yx;

//     return mix( tex.x, tex.y, u.z ) * 2.0 - 1.0;
// }


float fbm(vec3 p) {
    vec3 q = p + ps.time * 0.5 * vec3(1.0, -0.2, -1.0);
    float g = noise(q);

    float f = 0.0;
    float scale = ps.fbm_scale;
    float factor = ps.fbm_factor;

    for (int i = 0; i < 6; i++) {
        f += scale * noise(q);
        q *= factor;
        factor += 0.21;
        scale *= 0.5;
    }

    return f;
}



float map(vec3 p)
{   
    p.y -= ps.height;
    float distance = sd_box(p, vec3(10.0, 0.5, 10.0));

    float noise = fbm(p);
    return -distance + noise;
}

float map_sun(vec3 p)
{   
    p += descriptor_set_0_ubo.directional_light_direction;
    float distance = sd_sphere(p, 0.3);

    return -distance ;
}


void process_sky()
{
    vec2 frag_coord = fs_uv_non_scaled * 2.0 - 1.0;
    frag_coord.y = -frag_coord.y;


    vec3 ro = vec3(0, 0, 0);
    vec3 rd = normalize(vec3(frag_coord, -1)) * mat3(descriptor_set_0_ubo.view);

    

    float t = 0;
    vec3 color1 = vec3(1.0,1.0,1.0);
    vec3 color2 = vec3(0.4,0.6,0.7);
    vec4 res = vec4(mix(color1, color2, (rd.y + 1.0) * 0.5), 0.6);
    for(int i = 0; i < 10; i++)
    {
        vec3 p = ro + rd * t;
        
        float d = map(p);
        //float d_sun = map_sun(p);

        if (d > 0.0) {
            float diffuse = clamp((map(p) - map(p + 0.3 * -descriptor_set_0_ubo.directional_light_direction)) / 0.3, 0.0, 1.0 );

            vec3 lin = color2 * 1.1 + 0.8 * color1 * diffuse;
            vec4 color = vec4(mix(vec3(1.0,1.0,1.0), vec3(0.0, 0.0, 0.0), d), d );
            color.rgb *= lin;
            color.rgb *= color.a;
            res += color * (1.0 - res.a);
        }
        
        // if(d_sun > 0.0)
        // {
        //     vec4 color = vec4(mix(vec3(1.0,1.0,1.0), vec3(0.0, 0.0, 0.0), d_sun), 0.5 );
        //     color.rgb *= color.a;
        //     res += color * (1.0 - res.a);
        // }

        

        t += 0.2;
    }



    out_color = res;
}

void main() 
{
    uint stencil_id = texture(g_buffer_stencil, fs_uv).r;
    if(stencil_id == 0)
    {
        process_sky();
    }
    else
    {
        process_ambient_light();
    }
}