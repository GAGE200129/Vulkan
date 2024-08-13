#version 460 core
#extension GL_ARB_shading_language_include : require



layout(location = 0) in vec3 in_pos;
layout(location = 1) in uvec4 in_bone_ids;
layout(location = 2) in vec4 in_weights;

layout(push_constant, std140) uniform PushConstant {
    mat4x4 model_transform;
};

layout(set = 1, binding = 0) uniform Animation
{
    mat4x4 bone_matrices[100];
    bool enabled; 
} animation;


void main()
{
    vec4 total_position = model_transform * vec4(in_pos, 1.0);
    if(animation.enabled)
    {
        total_position = vec4(0, 0, 0, 0);
        for(uint i = 0 ; i < 4 ; i++)
        {
            vec4 local_position = animation.bone_matrices[in_bone_ids[i]] * vec4(in_pos,1.0f);
            total_position += local_position * in_weights[i];
        }
    }

    gl_Position = total_position;
}