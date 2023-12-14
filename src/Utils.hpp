#pragma once

namespace Utils
{
  glm::mat4x4 aiToGlmMatrix4x4(const aiMatrix4x4 &a);
  glm::quat aiToGlmQuaternion(const aiQuaternion &a);
  glm::vec3 aiToGlmVec3(const aiVector3D &a);
}
