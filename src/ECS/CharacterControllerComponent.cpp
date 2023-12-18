#include "pch.hpp"
#include "CharacterControllerComponent.hpp"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include "Bullet/BulletEngine.hpp"

#include <glm/gtx/string_cast.hpp>

class FindGround : public btCollisionWorld::ContactResultCallback
{
public:
  btScalar addSingleResult(btManifoldPoint &cp,
                           const btCollisionObjectWrapper *colObj0, int partId0, int index0,
                           const btCollisionObjectWrapper *colObj1, int partId1, int index1)
  {
    if (colObj0->m_collisionObject == mMe && !mHaveGround)
    {
      const btTransform &transform = mMe->getWorldTransform();
      btMatrix3x3 invBasis = transform.getBasis().transpose();
      btVector3 localPoint = invBasis * (cp.m_positionWorldOnB - transform.getOrigin());
      localPoint[2] += CharacterControllerComponent::HEIGHT;
      float r = localPoint.length();
      float cosTheta = localPoint[2] / r;

      if (std::fabs(r - CharacterControllerComponent::RADIUS) <= CharacterControllerComponent::RADIUS_THRESHOLD 
        && cosTheta < CharacterControllerComponent::MAX_COS_GROUND)
      {
        mHaveGround = true;
        mGroundPoint = cp.m_positionWorldOnB;
      }
    }
    return 0;
  }

  btRigidBody *mMe;
  // Assign some values, in some way
  bool mHaveGround = false;
  btVector3 mGroundPoint;
};

void CharacterControllerComponent::init()
{
  mTransform = mGameObject->getRequiredComponent<TransformComponent>();

  mShape = new btCapsuleShape(RADIUS, HEIGHT);
  btTransform t;
  t.setIdentity();
  t.setOrigin(btVector3(mTransform->position.x, mTransform->position.y, mTransform->position.z));
  mMotionState = new btDefaultMotionState(t);

  btRigidBody::btRigidBodyConstructionInfo info(4000.0f, mMotionState, mShape);
  mBody = new btRigidBody(info);
  mBody->setSleepingThresholds(0.0, 0.0);
  mBody->setAngularFactor(0.0);
  mBody->setFriction(0);
  BulletEngine::sDynamicWorld->addRigidBody(mBody);
}
void CharacterControllerComponent::update(float delta)
{
  FindGround callback;
  callback.mMe = mBody;
  BulletEngine::sDynamicWorld->contactTest(mBody, callback);
  mOnGround = callback.mHaveGround;
  mGroundPoint = glm::vec3{callback.mGroundPoint.x(), callback.mGroundPoint.y(), callback.mGroundPoint.z()};

  const btTransform &transform = mBody->getWorldTransform();
  btMatrix3x3 basis = transform.getBasis();
  btMatrix3x3 invBasis = basis.transpose();
  btVector3 linearVelocity = invBasis * mBody->getLinearVelocity();
  btVector3 moveDirection = btVector3(mMoveDirection.x, mMoveDirection.y, mMoveDirection.z);
  if (moveDirection.fuzzyZero() && mOnGround)
  {
    linearVelocity *= SPEED_DAMPING;
  }
  else if (mOnGround || linearVelocity[2] > 0)
  {
    linearVelocity += moveDirection * (WALK_ACCEL * delta);
    btScalar speed2 = pow(linearVelocity.x(), 2) + pow(linearVelocity.z(), 2);
    constexpr float MAX_VELOCITY_SQUARED = MAX_VELOCITY * MAX_VELOCITY;
    if (speed2 > MAX_VELOCITY_SQUARED)
    {
      btScalar correction = sqrt(MAX_VELOCITY_SQUARED / speed2);
      linearVelocity[0] *= correction;
      linearVelocity[2] *= correction;
    }
  }

  mBody->setLinearVelocity(basis * linearVelocity);
  btTransform &t = mBody->getWorldTransform();
  btVector3 &pos = t.getOrigin();
  btQuaternion rot = t.getRotation().normalized();

  mTransform->position = {pos.x(), pos.y(), pos.z()};
  mTransform->rotation.x = rot.x();
  mTransform->rotation.y = rot.y();
  mTransform->rotation.z = rot.z();
  mTransform->rotation.w = rot.w();
}

void CharacterControllerComponent::registerLuaScript(lua_State *L)
{
  auto setMoveDirection = [](lua_State *L) -> int
  {
    CharacterControllerComponent *c = (CharacterControllerComponent *)lua_touserdata(L, 1);
    glm::vec3 v;
    v.x = lua_tonumber(L, 2);
    v.y = lua_tonumber(L, 3);
    v.z = lua_tonumber(L, 4);
    c->mMoveDirection = v;
    return 0;
  };

  lua_register(L, "character_controller_set_move_dir", setMoveDirection);
}
void CharacterControllerComponent::shutdown() noexcept
{
  BulletEngine::sDynamicWorld->removeRigidBody(mBody);
  delete mMotionState;
  delete mShape;
  delete mBody;
}