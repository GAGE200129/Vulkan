#include "pch.hpp"
#include "RigidBodyComponent.hpp"

#include "Bullet/BulletEngine.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

void RigidBodyComponent::init()
{
  mTransform = mGameObject->getRequiredComponent<TransformComponent>();
  mShape = mGameObject->getRequiredComponent<CollisionShapeBase>();

  btVector3 intertia;
  mShape->mShape->calculateLocalInertia(mMass, intertia);
  btTransform t;
  t.setIdentity();
  t.setOrigin(btVector3(mTransform->position.x, mTransform->position.y, mTransform->position.z));
  mMotionState = new btDefaultMotionState(t);
  
  btRigidBody::btRigidBodyConstructionInfo info(mMass, mMotionState, mShape->mShape, intertia);
  mBody = new btRigidBody(info);
  mBody->setFriction(1.0);
  BulletEngine::sDynamicWorld->addRigidBody(mBody);
}

void RigidBodyComponent::update(float delta)
{
  btTransform& t = mBody->getWorldTransform();
  btVector3& pos = t.getOrigin();
  btQuaternion rot = t.getRotation().normalized();

  mTransform->rotation.x = rot.x();
  mTransform->rotation.y = rot.y();
  mTransform->rotation.z = rot.z();
  mTransform->rotation.w = rot.w();

  glm::vec3 offsetRotation = glm::mat3(mTransform->rotation) * mShape->mColliderOffset; 
  mTransform->position = {
    pos.x() - offsetRotation.x,
    pos.y() - offsetRotation.y,
    pos.z() - offsetRotation.z
  };
}

void RigidBodyComponent::shutdown() noexcept
{
  BulletEngine::sDynamicWorld->removeRigidBody(mBody);
  delete mBody;
  delete mMotionState;
}