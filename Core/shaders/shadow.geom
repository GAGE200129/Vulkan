#version 460 core
#extension GL_ARB_shading_language_include : require

#include "includes/descriptor_set_0.inc"
    
layout(triangles, invocations = CASCADE_COUNT) in;
layout(triangle_strip, max_vertices = 3) out;
    
    
void main()
{          
    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        gl_Position = descriptor_set_0_ubo.directional_light_proj_views[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive(); 
}  