#include "BodySetup.h"

#include "PhysicalMaterial.h"
#include "UObject/ObjectFactory.h"

float UBodySetup::CalculateMass() const
{
    return 0.0f; 
}

void UBodySetup::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);

    Ar << MassInKg << bGenerateMirroredCollision << AggGeom;
    
    if (Ar.IsSaving())
    {
        if (!PhysicalMaterial)
        {
            PhysicalMaterial = FObjectFactory::ConstructObject<UPhysicalMaterial>(nullptr);
        }
        FName PhysicalMaterialName = PhysicalMaterial->GetFName();
        Ar << PhysicalMaterialName;
    }
    else
    {
        FName PhysicalMaterialName;
        Ar << PhysicalMaterialName;
        PhysicalMaterial = FObjectFactory::ConstructObject<UPhysicalMaterial>(nullptr, PhysicalMaterialName);
    }
    
    PhysicalMaterial->SerializeAsset(Ar);

    
}
