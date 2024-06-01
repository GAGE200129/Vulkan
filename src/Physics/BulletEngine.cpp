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

static btStaticPlaneShape* gPlaneShape = nullptr;
static btCollisionObject* gPlaneObject = nullptr;

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

    gPlaneShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btTransform t;
    t.setOrigin(btVector3(0, 0, 0));
    t.setRotation(btQuaternion(0, 0, 0, 1));
    gPlaneObject = new btCollisionObject();
    gPlaneObject->setWorldTransform(t);
    gPlaneObject->setCollisionShape(gPlaneShape);
    gPlaneObject->setFriction(1.0);

    sDynamicWorld->addCollisionObject(gPlaneObject);
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

    sDynamicWorld->removeCollisionObject(gPlaneObject);
    delete gPlaneShape;
    delete gPlaneObject;
    
    delete sDynamicWorld;
    delete sSolverPool;
    delete sSolver;
    delete sBroadphaseInterface;
    delete sDispatcher;
    delete sCollisionConfiguration;
}