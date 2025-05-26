#include "BodyInstance.h"

#include "BodySetup.h"
#include "PhysicsAssetUtils.h"
#include "PhysXManager.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicalMaterial.h"

void FBodyInstance::InitBody(PxScene* Scene, class UPrimitiveComponent* Owner)
{
    if (bInitialized)
        return;

    if (!BodySetup || !Scene)
        return;

    PxPhysics* Physics = FPhysXManager::Get().GetPhysics();
    if (!Physics)
        return;

    const FKAggregateGeom& AggGeom = BodySetup->GetAggGeom();
    if (AggGeom.BoxElems.Num() == 0 && 
        AggGeom.SphereElems.Num() == 0 &&
        AggGeom.SphylElems.Num() == 0 &&
        AggGeom.ConvexElems.Num() == 0)
    {
        return; // No shapes to create
    }

    const FTransform WorldTM = Owner->GetComponentToWorld();
    const PxTransform PxTM = ToPxTransform(WorldTM);

    RigidBody = Physics->createRigidDynamic(PxTM);
    if (!RigidBody)
    {
        UE_LOG(ELogLevel::Warning, TEXT("Failed to create PxRigidDynamic for BodyInstance."));
        return;
    }
    
    PxMaterial* Material = BodySetup->PhysicalMaterial ? 
        BodySetup->PhysicalMaterial->GetOrCreateNativeMaterial() : 
        FPhysXManager::Get().GetDefaultMaterial();

    if (!Material)
    {
        UE_LOG(ELogLevel::Warning, TEXT("Failed to create PxMaterial for BodyInstance."));
        return;
    }

    for (const FKBoxElem& BoxElem : AggGeom.BoxElems)
    {
        PxBoxGeometry BoxGeom(BoxElem.X, BoxElem.Y, BoxElem.Z);
        PxShape* Shape = Physics->createShape(BoxGeom, *Material);
        if (!Shape)
        {
            continue;
        }
        
        FTransform BoxLoaclTM = BoxElem.GetTransform();
        PxTransform BoxLocalPose = ToPxTransform(BoxLoaclTM);

        Shape->setLocalPose(BoxLocalPose);
        RigidBody->attachShape(*Shape);

        Shape->release();
    }

    PxRigidBodyExt::updateMassAndInertia(*RigidBody, BodySetup->MassInKg > 0 ? 
        BodySetup->MassInKg : 1.0f);

    Scene->addActor(*RigidBody);

    PxRigidActor = RigidBody;
    // PxScene = Scene;
    bInitialized = true;
}
