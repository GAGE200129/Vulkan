#pragma once




namespace JPH
{
    class BodyInterface;
    class PhysicsSystem;
    class TempAllocatorImpl;
    class JobSystemThreadPool;
}

namespace gage::phys
{
    class BPLayerInterface;
    class ObjectVsBroadPhaseLayerFilter;
    class ObjectLayerPairFilter;
    class Physics
    {
    public:
        Physics();
        ~Physics();

        void update(float delta);
    private:
        JPH::BodyInterface *p_body_interface{};
        std::unique_ptr<JPH::PhysicsSystem> physics_system;
        std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
        std::unique_ptr<JPH::JobSystemThreadPool> job_system;
        std::unique_ptr<BPLayerInterface> broad_phase_layer_interface;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilter> object_vs_broadphase_layer_filter;
        std::unique_ptr<ObjectLayerPairFilter> object_vs_object_layer_filter;
    };
}