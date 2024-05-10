#include "pch.hpp"
#include "Utils.hpp"

glm::mat4x4 Utils::aiToGlmMatrix4x4(const aiMatrix4x4 &a)
{
    glm::mat4x4 b;
    b[0][0] = a.a1;
    b[1][0] = a.a2;
    b[2][0] = a.a3;
    b[3][0] = a.a4;
    b[0][1] = a.b1;
    b[1][1] = a.b2;
    b[2][1] = a.b3;
    b[3][1] = a.b4;
    b[0][2] = a.c1;
    b[1][2] = a.c2;
    b[2][2] = a.c3;
    b[3][2] = a.c4;
    b[0][3] = a.d1;
    b[1][3] = a.d2;
    b[2][3] = a.d3;
    b[3][3] = a.d4;
    return b;
}

glm::quat Utils::aiToGlmQuaternion(const aiQuaternion &a)
{
    glm::quat result;
    result.w = a.w;
    result.x = a.x;
    result.y = a.y;
    result.z = a.z;

    return result;
}

glm::vec3 Utils::aiToGlmVec3(const aiVector3D &a)
{
    glm::vec3 result;
    result.x = a.x;
    result.y = a.y;
    result.z = a.z;
    return result;
}