#pragma once

class BulletEngine
{
public:
    static void init();
    static void update();
    static void cleanup();


    static btRigidBody* createRigidBody(btRigidBody::btRigidBodyConstructionInfo info);

    inline static btDynamicsWorld* getWorld() { return sDynamicWorld; }
private:
    static btCollisionConfiguration *sCollisionConfiguration;
    static btCollisionDispatcher *sDispatcher;
    static btBroadphaseInterface *sBroadphaseInterface;
    static btConstraintSolver *sSolver;
    static btConstraintSolverPoolMt *sSolverPool;
    static btDynamicsWorld *sDynamicWorld;

    static std::vector<btRigidBody*> sRigidBodies;
};