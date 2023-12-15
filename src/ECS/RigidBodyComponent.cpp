#include "pch.hpp"
#include "RigidBodyComponent.hpp"

#include "Bullet/BulletEngine.hpp"

#include <glm/gtx/string_cast.hpp>

void RigidBodyComponent::init()
{
  mTransform = mGameObject->getRequiredComponent<TransformComponent>();

  btVector3 intertia;
  mShape = new btSphereShape(1);
  mShape->calculateLocalInertia(1.0f, intertia);
  btTransform t;
  t.setOrigin(btVector3(mTransform->position.x, mTransform->position.y, mTransform->position.z));
  mMotionState = new btDefaultMotionState(t);
  
  btRigidBody::btRigidBodyConstructionInfo info(1.0f, mMotionState, mShape, intertia);
  mBody = new btRigidBody(info);
  BulletEngine::sDynamicWorld->addRigidBody(mBody);

  mBody->activate();
  mBody->applyCentralImpulse(btVector3(3, 0, 1));
}

void RigidBodyComponent::update(float delta)
{
  btTransform& t = mBody->getWorldTransform();
  btVector3& pos = t.getOrigin();
  btQuaternion rot = t.getRotation().normalized();
  
  mTransform->position = {pos.x(), pos.y(), pos.z()};
  mTransform->rotation.x = rot.x();
  mTransform->rotation.y = rot.y();
  mTransform->rotation.z = rot.z();
  mTransform->rotation.w = rot.w();
}

void RigidBodyComponent::shutdown() noexcept
{
  delete mBody;
  delete mMotionState;
  delete mShape;
}