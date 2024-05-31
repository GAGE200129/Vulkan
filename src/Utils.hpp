#pragma once

namespace Utils
{

    btVector3 glmToBtVec3(const glm::vec3& a);

    glm::vec3 btToGlmVec3(const btVector3 &a);

    glm::mat4x4 aiToGlmMatrix4x4(const aiMatrix4x4 &a);
    glm::quat aiToGlmQuaternion(const aiQuaternion &a);
    glm::vec3 aiToGlmVec3(const aiVector3D &a);

    bool filePathToVectorOfChar(const std::string& filePath, std::vector<char>& v);
}
