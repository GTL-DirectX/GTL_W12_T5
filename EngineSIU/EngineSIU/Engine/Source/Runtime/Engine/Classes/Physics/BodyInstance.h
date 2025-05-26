#pragma once

#include "ShapeElem.h"
#include "Hal/PlatformType.h"
#include "UObject/ObjectMacros.h"

#include <PxPhysicsAPI.h>

class UPhysicalMaterial;
class UBodySetup;

using namespace physx;

struct FBodyInstanceCore
{
    DECLARE_STRUCT(FBodyInstanceCore)

    // 해당 BodyInstance를 Dynamic으로 생성할지 Static으로 생성할지 여부
    UPROPERTY(EditAnywhere, bool, bSimulatePhysics, = true)

    // 충돌 처리 여부 (Query, Physics, Both)
    ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    // 질량 (자동 계산 시 무시)
    UPROPERTY(EditAnywhere, float, MassInKgOverride, = -1.0f)
    
};


struct FBodyInstance : public FBodyInstanceCore
{
    DECLARE_STRUCT(FBodyInstance, FBodyInstanceCore)

    int32 InstanceBodyIndex = INDEX_NONE;
    
    PxRigidActor* PxRigidActor = nullptr; // Pointer to the PhysX rigid actor that this instance is associated with

    UBodySetup* BodySetup = nullptr; // BodySetupCore pointer that this instance is initialized from

    UPhysicalMaterial* PhysMaterial = nullptr; // Physical material associated with this body instance

    // 현재 Transform 상태 (월드 좌표계 기준)
    PxTransform BodyToWorld;

    FVector LinearVelocity = FVector::ZeroVector; // Linear velocity of the body instance
    FVector AngularVelocity = FVector::ZeroVector; // Angular velocity of the body instance

    bool bInitialized = false;
    bool bStartAwake = true; // Whether the body instance should start awake
    float ComputedMAss = 0.0f; // Computed mass of the body instance, if not overridden

    // Body 생성
    void InitBody(PxScene* Scene, class UPrimitiveComponent* Owner);

    // 힘/속도/토크 API
    void AddForce(const FVector& Force);
    void AddTorque(const FVector& Torque);
    void SetLinearVelocity(const FVector& Velocity);
    void SetAngularVelocity(const FVector& Velocity);
    FVector GetLinearVelocity() const;
    FVector GetAngularVelocity() const;

    // Transform 갱신
    void SetBodyTransform(const FTransform& NewTransform);
    FTransform GetBodyTransform() const;

    // 종료
    void TermBody();
    
};


