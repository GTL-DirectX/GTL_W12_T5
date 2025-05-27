#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "AggregateGeom.h"

class UPhysicalMaterial;
// Skeletal Mesh 본에 매핑되는 물리 본체 구성에 사용.
// 물리 본체는 물리 시뮬레이션을 위해 사용되는 물리적 본체를 정의.
// 각 본마다 하나의 UBodySetup이 존재.
class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)

public:
    UBodySetupCore() = default;
    // ECollisionTraceFlag CollisionTraceFlag;

    FName BoneName;

    virtual void Serialize(FArchive& Ar) override
    {
        Super::Serialize(Ar);
        Ar << BoneName;
    }
    
};

class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)

public:
    UBodySetup() = default;

    const FKAggregateGeom& GetAggGeom() const { return AggGeom; }

    // TODO: 테스트용 하드코딩 함수. 제거할 것.
    void AddBoxElem(const FKBoxElem& BoxElem) { AggGeom.BoxElems.Add(BoxElem); }
    void AddSphereElem(const FKSphereElem& SphereElem) { AggGeom.SphereElems.Add(SphereElem); }

    UPhysicalMaterial* PhysicalMaterial = nullptr;

    float MassInKg = 0.0f;
    bool bGenerateMirroredCollision = false;

    float CalculateMass() const;

    virtual void Serialize(FArchive& Ar) override;
    
    
private:
    FKAggregateGeom AggGeom;
    
};
