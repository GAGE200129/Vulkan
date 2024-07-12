#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT

#include "pbr_functions.inc"
#include "material.inc"

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
};

vec3 calculate_directional_light_phong(in DirectionalLight light, in vec3 n, in vec3 to_cam_dir, in vec3 albedo)
{
    //Diffuse light
    float diffuse_factor = max(dot(n, -light.direction), 0.0);
    vec3 diffuse_result;
    if(diffuse_factor > 0)
    {
        diffuse_result = light.color * 1.0 /*Light intensity*/ * diffuse_factor * material.color.rgb;
    }
    //Specular

    

    return diffuse_result * albedo;
}




#endif