#include "pch.hpp"
#include "RigidBodyComponent.hpp"

#include "Physics/BulletEngine.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "GameObject.hpp"

void RigidBodyComponent::init()
{
    mShape = mGameObject->getRequiredComponent<CollisionShapeBase>();

    btVector3 intertia;
    mShape->mShape->calculateLocalInertia(mMass, intertia);
    btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(mGameObject->mPosition.x, mGameObject->mPosition.y, mGameObject->mPosition.z));
    mMotionState = new btDefaultMotionState(t);

    btRigidBody::btRigidBodyConstructionInfo info(mMass, mMotionState, mShape->mShape, intertia);
    mBody = new btRigidBody(info);
    mBody->setFriction(1.0);
    BulletEngine::sDynamicWorld->addRigidBody(mBody);
}

void RigidBodyComponent::update()
{
    btTransform &t = mBody->getWorldTransform();
    btVector3 &pos = t.getOrigin();
    btQuaternion rot = t.getRotation().normalized();

    mGameObject->mRotation.x = rot.x();
    mGameObject->mRotation.y = rot.y();
    mGameObject->mRotation.z = rot.z();
    mGameObject->mRotation.w = rot.w();

    glm::vec3 offsetRotation = glm::mat3(mGameObject->mRotation) * mShape->mColliderOffset;
    mGameObject->mPosition = {
        pos.x() - offsetRotation.x,
        pos.y() - offsetRotation.y,
        pos.z() - offsetRotation.z};
}

void RigidBodyComponent::shutdown() noexcept
{
    BulletEngine::sDynamicWorld->removeRigidBody(mBody);
    delete mBody;
    delete mMotionState;
}