#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

namespace physx
{
    class PxMaterial;
}

// 표면 물리 특성을 정의하는 클래스.
class UPhysicalMaterial : public UObject
{
    DECLARE_CLASS(UPhysicalMaterial, UObject)
    
public:
    UPhysicalMaterial() = default;
    virtual ~UPhysicalMaterial() = default;


    physx::PxMaterial* GetOrCreateNativeMaterial();

    float GetStaticFriction() const { return StaticFriction; }
    float GetDynamicFriction() const { return DynamicFriction; }
    float GetRestitution() const { return Restitution; }
    void SetStaticFriction(float InStaticFriction);
    void SetDynamicFriction(float InDynamicFriction);
    void SetRestitution(float InRestitution);

    virtual void Serialize(FArchive& Ar) override;

private:
    physx::PxMaterial* CachedMaterial = nullptr;
    
    float StaticFriction = 1.0f;
    float DynamicFriction = 1.0f;
    float Restitution = 0.0f;
    
};
