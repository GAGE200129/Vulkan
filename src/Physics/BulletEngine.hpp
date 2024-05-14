#pragma once

#include "btBulletDynamicsCommon.h"
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>

class BulletEngine
{
    friend class RigidBodyComponent;
    friend class CharacterControllerComponent;

public:
    static void init();
    static void update();
    static void cleanup();

private:
    static btCollisionConfiguration *sCollisionConfiguration;
    static btCollisionDispatcher *sDispatcher;
    static btBroadphaseInterface *sBroadphaseInterface;
    static btConstraintSolver *sSolver;
    static btConstraintSolverPoolMt *sSolverPool;
    static btDynamicsWorld *sDynamicWorld;
    static btCollisionShape *sGlobalPlaneCollision;
    static btCollisionObject *sGlobalPlane;
};