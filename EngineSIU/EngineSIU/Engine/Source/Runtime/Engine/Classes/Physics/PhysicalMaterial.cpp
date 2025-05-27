#include "PhysicalMaterial.h"

#include "PhysXManager.h"

physx::PxMaterial* UPhysicalMaterial::GetOrCreateNativeMaterial()
{
    if (!CachedMaterial)
    {
        CachedMaterial = FPhysXManager::Get().GetPhysics()->createMaterial(StaticFriction, DynamicFriction, Restitution);
    }
    return CachedMaterial;
}

void UPhysicalMaterial::SetStaticFriction(float InStaticFriction)
{
    StaticFriction = InStaticFriction;
    if (CachedMaterial)
    {
        CachedMaterial->setStaticFriction(InStaticFriction);
    }
}

void UPhysicalMaterial::SetDynamicFriction(float InDynamicFriction)
{
    DynamicFriction = InDynamicFriction;
    if (CachedMaterial)
    {
        CachedMaterial->setDynamicFriction(InDynamicFriction);
    }
}

void UPhysicalMaterial::SetRestitution(float InRestitution)
{
    Restitution = InRestitution;
    if (CachedMaterial)
    {
        CachedMaterial->setRestitution(InRestitution);
    }
}

void UPhysicalMaterial::Serialize(FArchive& Ar)
{
    UObject::Serialize(Ar);
    Ar << StaticFriction;
    Ar << DynamicFriction;
    Ar << Restitution;
}
