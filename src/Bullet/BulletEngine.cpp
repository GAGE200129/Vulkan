#include "pch.hpp"
#include "BulletEngine.hpp"


btCollisionConfiguration* BulletEngine::sCollisionConfiguration;
btCollisionDispatcher*    BulletEngine::sDispatcher;
btBroadphaseInterface*    BulletEngine::sBroadphaseInterface;
btConstraintSolver*       BulletEngine::sSolver;
btConstraintSolverPoolMt* BulletEngine::sSolverPool;
btDynamicsWorld*          BulletEngine::sDynamicWorld;
btCollisionShape*         BulletEngine::sGlobalPlaneCollision;
btCollisionObject*        BulletEngine::sGlobalPlane;
void BulletEngine::init()
{
  sCollisionConfiguration = new btDefaultCollisionConfiguration();
  sDispatcher = new btCollisionDispatcher(sCollisionConfiguration);
  sBroadphaseInterface = new btDbvtBroadphase();
  sSolver = new btSequentialImpulseConstraintSolver();
  sSolverPool = new btConstraintSolverPoolMt(4);
  sDynamicWorld = new btDiscreteDynamicsWorldMt(sDispatcher, sBroadphaseInterface, sSolverPool, sSolver, sCollisionConfiguration);
  sDynamicWorld->setGravity(btVector3(0, -9.8, 0));

  sGlobalPlaneCollision = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
  sGlobalPlane = new btCollisionObject();
  sGlobalPlane->setCollisionShape(sGlobalPlaneCollision); 

  sDynamicWorld->addCollisionObject(sGlobalPlane);

}

void BulletEngine::update(float delta)
{
  sDynamicWorld->stepSimulation(delta);
}

void BulletEngine::cleanup()
{
  delete sGlobalPlane;
  delete sGlobalPlaneCollision;
  delete sDynamicWorld;
  delete sSolver;
  delete sBroadphaseInterface;
  delete sDispatcher;
  delete sCollisionConfiguration;
}