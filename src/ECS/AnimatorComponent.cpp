#include "pch.hpp"
#include "AnimatorComponent.hpp"

void AnimatorComponent::init()
{
  mAnimationTimeSec = 0.0f;
  mCurrentAnimation = nullptr;
  mAnimatedModel = mGameObject->getRequiredComponent<AnimatedModelComponent>();
  mTransformComponent = mGameObject->getRequiredComponent<TransformComponent>();
}


void AnimatorComponent::setCurrentAnimation(const std::string& name)
{
  aiAnimation** animations = mAnimatedModel->mMeshData->mPScene->mAnimations;
  const unsigned int animation_count = mAnimatedModel->mMeshData->mPScene->mNumAnimations;
  for(unsigned int i = 0 ;i < animation_count; i++)
  {
    const aiAnimation* animation = animations[i];
    if(std::strcmp(animation->mName.C_Str(), name.c_str()) == 0)
    {
      mCurrentAnimation = animation;
    }
  }
}
void AnimatorComponent::update(float delta)
{
  mAnimationTimeSec += delta;

  if (mCurrentAnimation != nullptr)
  {
    double ticksPerSeconds = mCurrentAnimation->mTicksPerSecond != 0 ? mCurrentAnimation->mTicksPerSecond : 25.0;
    mAnimationTimeTick = std::fmod(mAnimationTimeSec * ticksPerSeconds, mCurrentAnimation->mDuration);
  }

  // Update mBoneTransform of animated model
  AnimatedModelComponent::MeshData *meshData = mAnimatedModel->mMeshData;

  std::function<void(const aiNode *node, glm::mat4x4 &accumulatedTransform)> parse_node;
  parse_node = [&](const aiNode *node, glm::mat4x4 &accumulatedTransform)
  {
    std::string nodeName = node->mName.C_Str();
    glm::mat4x4 nodeTransform = Utils::aiToGlmMatrix4x4(node->mTransformation);

    const aiNodeAnim *pNodeAnim = findNodeAnim(nodeName);

    if (pNodeAnim)
    {
      glm::quat rotation = calculateInterpolatedRotation(pNodeAnim);
      glm::vec3 position = calculateInterpolatedPosition(pNodeAnim);
      glm::vec3 scale = calculateInterpolatedScale(pNodeAnim);

      nodeTransform = glm::translate(glm::mat4x4(1.0f), position) * glm::scale(glm::mat4x4(1.0f), scale) * glm::mat4(rotation);
    }

    accumulatedTransform *= nodeTransform;
    std::vector<glm::mat4x4> &boneIndexToOffset = meshData->mBoneIndexToOffset;
    std::map<std::string, unsigned int> boneNameToIndex = meshData->mBoneNameToIndex;
    if (boneNameToIndex.find(nodeName) != boneNameToIndex.end())
    {
      unsigned int boneIndex = boneNameToIndex.at(nodeName);
      mAnimatedModel->mBoneTransforms[boneIndex] = accumulatedTransform * boneIndexToOffset.at(boneIndex);
    }

    for (int i = 0; i < node->mNumChildren; i++)
    {
      parse_node(node->mChildren[i], accumulatedTransform);
    }
  };
  glm::mat4x4 mat = glm::mat4x4(1.0f);
  parse_node(mAnimatedModel->mMeshData->mPScene->mRootNode, mat);
}

const aiNodeAnim *AnimatorComponent::findNodeAnim(const std::string &nodeName)
{
  if(!mCurrentAnimation)
    return nullptr;
  
  for (unsigned int i = 0; i < mCurrentAnimation->mNumChannels; i++)
  {
    const aiNodeAnim *pNodeAnim = mCurrentAnimation->mChannels[i];
    if (std::strcmp(pNodeAnim->mNodeName.C_Str(), nodeName.c_str()) == 0)
    {
      return pNodeAnim;
    }
  }

  return nullptr;
}

glm::vec3 AnimatorComponent::calculateInterpolatedScale(const aiNodeAnim *pNodeAnim)
{
  if (pNodeAnim->mNumScalingKeys <= 1)
  {
    return Utils::aiToGlmVec3(pNodeAnim->mScalingKeys[0].mValue);
  }

  unsigned int index;
  for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
  {
    double t = pNodeAnim->mScalingKeys[i + 1].mTime;
    if (mAnimationTimeTick < t)
    {
      index = i;
      break;
    }
  }
  double t1 = pNodeAnim->mScalingKeys[index].mTime;
  double t2 = pNodeAnim->mScalingKeys[index + 1].mTime;
  float delta = (mAnimationTimeTick - t1) / (t2 - t1);
  glm::vec3 s1 = Utils::aiToGlmVec3(pNodeAnim->mScalingKeys[index].mValue);
  glm::vec3 s2 = Utils::aiToGlmVec3(pNodeAnim->mScalingKeys[index + 1].mValue);

  return glm::mix(s1, s2, delta);
}

glm::quat AnimatorComponent::calculateInterpolatedRotation(const aiNodeAnim *pNodeAnim)
{
  if (pNodeAnim->mNumRotationKeys <= 1)
  {
    return Utils::aiToGlmQuaternion(pNodeAnim->mRotationKeys[0].mValue);
  }

  unsigned int index;
  for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
  {
    double t = pNodeAnim->mRotationKeys[i + 1].mTime;
    if (mAnimationTimeTick < t)
    {
      index = i;
      break;
    }
  }
  double t1 = pNodeAnim->mRotationKeys[index].mTime;
  double t2 = pNodeAnim->mRotationKeys[index + 1].mTime;
  float delta = (mAnimationTimeTick - t1) / (t2 - t1);
  glm::quat q1 = Utils::aiToGlmQuaternion(pNodeAnim->mRotationKeys[index].mValue);
  glm::quat q2 = Utils::aiToGlmQuaternion(pNodeAnim->mRotationKeys[index + 1].mValue);

  return glm::slerp(q1, q2, delta);
}

glm::vec3 AnimatorComponent::calculateInterpolatedPosition(const aiNodeAnim *pNodeAnim)
{
  if (pNodeAnim->mNumPositionKeys <= 1)
  {
    return Utils::aiToGlmVec3(pNodeAnim->mPositionKeys[0].mValue);
  }

  unsigned int index;
  for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
  {
    double t = pNodeAnim->mPositionKeys[i + 1].mTime;
    if (mAnimationTimeTick < t)
    {
      index = i;
      break;
    }
  }
  double t1 = pNodeAnim->mPositionKeys[index].mTime;
  double t2 = pNodeAnim->mPositionKeys[index + 1].mTime;
  double delta = (mAnimationTimeTick - t1) / (t2 - t1);
  glm::vec3 p1 = Utils::aiToGlmVec3(pNodeAnim->mPositionKeys[index].mValue);
  glm::vec3 p2 = Utils::aiToGlmVec3(pNodeAnim->mPositionKeys[index + 1].mValue);

  return glm::mix(p1, p2, (float)delta);
}
