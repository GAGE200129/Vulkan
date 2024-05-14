#pragma once

#include "Components.hpp"
#include "AnimatedModelComponent.hpp"

class AnimatorComponent : public Component
{
public:
  void init() override;
  void update() override;

  void setCurrentAnimation(const std::string &name);

private:
  const aiNodeAnim *findNodeAnim(const std::string &nodeName);
  glm::quat calculateInterpolatedRotation(const aiNodeAnim *pNodeAnim);
  glm::vec3 calculateInterpolatedPosition(const aiNodeAnim *pNodeAnim);
  glm::vec3 calculateInterpolatedScale(const aiNodeAnim *pNodeAnim);

private:
  double mAnimationTimeSec;
  double mAnimationTimeTick;
  const aiAnimation *mCurrentAnimation;
  AnimatedModelComponent *mAnimatedModel;
};