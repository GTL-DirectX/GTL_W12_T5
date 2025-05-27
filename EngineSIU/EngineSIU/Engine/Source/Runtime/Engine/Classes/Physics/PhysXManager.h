#pragma once

#include <PxPhysicsAPI.h>

using namespace physx;

class FPhysXManager
{
public:
    FPhysXManager();
    ~FPhysXManager();
    
public:
    static FPhysXManager& Get();

private:
    void InitPhysX();
    void ShutdonwPhysX();


public:
    PxPhysics* GetPhysics() const { return Physics; }
    PxCooking* GetCooking() const { return Cooking; }
    PxFoundation* GetFoundation() const { return Foundation; }
    PxDefaultCpuDispatcher* GetCpuDispatcher() const { return CpuDispatcher; }
    PxMaterial* GetDefaultMaterial() const { return DefaultMaterial; }
    
    PxScene* CreateScene();
    void DestroyScene(PxScene* Scene);
    
private:
    PxFoundation* Foundation = nullptr;
    PxCooking* Cooking = nullptr; // PhysX 조리 객체, 물리 형상을 생성하는 데 사용됨.
    PxPhysics* Physics = nullptr; // PhysX 기본 물리 엔진 객체.
    PxDefaultCpuDispatcher* CpuDispatcher = nullptr; // PhysX CPU 디스패처, 멀티스레딩을 지원하기 위해 사용됨.
    PxMaterial* DefaultMaterial = nullptr;
    
    PxPvd* PvdInstance = nullptr; // PhysX PVD (PhysX Visual Debugger), 디버깅을 위한 객체.
    PxPvdTransport* Transport = nullptr;
    
    PxDefaultAllocator Allocator; // PhysX 메모리 할당자.
    PxDefaultErrorCallback ErrorCallback; // PhysX 오류 콜백, 오류 발생 시 호출됨.
    
};
