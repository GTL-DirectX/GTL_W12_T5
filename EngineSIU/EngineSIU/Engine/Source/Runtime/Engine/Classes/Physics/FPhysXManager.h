#pragma once

#include <PxPhysicsAPI.h>

using namespace physx;

class FPhysXManager
{
public:
    static FPhysXManager& Get();

private:
    void InitPhysX();

private:
    PxPhysics* Physics = nullptr; // PhysX 기본 물리 엔진 객체. 
    
};
