layout(set = 2, binding = 0) uniform Animation
{
    mat4x4 bone_matrices[100];
    uint enabled;
} animation;
