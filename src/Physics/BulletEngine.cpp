#include "pch.hpp"
#include "BulletEngine.hpp"

#include "EngineConstants.hpp"

#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>

btCollisionConfiguration *BulletEngine::sCollisionConfiguration;
btCollisionDispatcher *BulletEngine::sDispatcher;
btBroadphaseInterface *BulletEngine::sBroadphaseInterface;
btConstraintSolver *BulletEngine::sSolver;
btConstraintSolverPoolMt *BulletEngine::sSolverPool;
btDynamicsWorld *BulletEngine::sDynamicWorld;
std::vector<btRigidBody*> BulletEngine::sRigidBodies;
std::vector<btCollisionObject*> BulletEngine::sCollisionObjects;

void BulletEngine::init()
{
    spdlog::info("Bullet engine init.");
    sCollisionConfiguration = new btDefaultCollisionConfiguration();
    sDispatcher = new btCollisionDispatcher(sCollisionConfiguration);
    sBroadphaseInterface = new btDbvtBroadphase();
    sSolver = new btSequentialImpulseConstraintSolver();
    sSolverPool = new btConstraintSolverPoolMt(2);
    sDynamicWorld = new btDiscreteDynamicsWorldMt(sDispatcher, sBroadphaseInterface, sSolverPool, sSolver, sCollisionConfiguration);
    sDynamicWorld->setGravity(btVector3(0, -9.8, 0));

    btStaticPlaneShape* planeShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btTransform t;
    t.setOrigin(btVector3(0, 0, 0));
    t.setRotation(btQuaternion(0, 0, 0, 1));
    createCollisionObject(planeShape, t);
}

void BulletEngine::update()
{
    sDynamicWorld->stepSimulation(EngineConstants::TICK_TIME, 0, EngineConstants::TICK_TIME);
}

btRigidBody* BulletEngine::createRigidBody(btRigidBody::btRigidBodyConstructionInfo info)
{
    btRigidBody* pBody = new btRigidBody(info);
    sDynamicWorld->addRigidBody(pBody);
    sRigidBodies.push_back(pBody);

    return pBody;
}

btCollisionObject* BulletEngine::createCollisionObject(btCollisionShape* shape, btTransform transform)
{
    btCollisionObject* object = new btCollisionObject();
    object->setWorldTransform(transform);
    object->setCollisionShape(shape);
    object->setFriction(1.0);

    sDynamicWorld->addCollisionObject(object);
    sCollisionObjects.push_back(object);

    return object;
}

void BulletEngine::cleanup()
{
    for(btRigidBody* rigidBody : sRigidBodies)
    {
        if(rigidBody->getMotionState())
            delete rigidBody->getMotionState();
        if(rigidBody->getCollisionShape())
            delete rigidBody->getCollisionShape();
    
        sDynamicWorld->removeRigidBody(rigidBody);
        delete rigidBody;
    }

    for(btCollisionObject* collisionObject : sCollisionObjects)
    {
        if(collisionObject->getCollisionShape())
            delete collisionObject->getCollisionShape();
        
        delete collisionObject;
    }
    delete sDynamicWorld;
    delete sSolverPool;
    delete sSolver;
    delete sBroadphaseInterface;
    delete sDispatcher;
    delete sCollisionConfiguration;
}