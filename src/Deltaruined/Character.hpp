#pragma once

#include <glm/vec3.hpp>

#include "Asset/StaticModelLoader.hpp"
#include "BulletDynamics/Dynamics/btRigidBody.h"

class Character
{
public:
    Character(const std::string &modelPath);

    void update();
    void render() const;

    inline const glm::vec3 getPosition() const
    {   
        btTransform& t = mBody->getWorldTransform();
        return Utils::btToGlmVec3(t.getOrigin());
    }

    inline void setMoveDirection(const glm::vec3& moveDirection) { mMoveDirection = moveDirection;}
    inline const btRigidBody* getBody() const { return mBody;}
    inline btRigidBody* getBody() { return mBody;}
private:
    glm::mat4x4 buildModelTransform() const;
    
private:
    btRigidBody *mBody;
    glm::vec3    mMoveDirection;
    StaticModelData *mModel;
};