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

    const FKAggregateGeom& AggGeom = BodySetup->AggGeom;
    if (AggGeom.BoxElems.Num() == 0 && 
        AggGeom.SphereElems.Num() == 0 &&
        AggGeom.SphylElems.Num() == 0 &&
        AggGeom.ConvexElems.Num() == 0)
    {
        return; // No shapes to create
    }

    const FTransform WorldTM = Owner->GetComponentTransform();
    const PxTransform PxTM = ToPxTransform(WorldTM);

    if (Owner->PhysicsBodyType == EPhysicsBodyType::Static)
    {
        PxRigidActor = Physics->createRigidStatic(PxTM);
    }
    else
    {
        RigidBody = Physics->createRigidDynamic(PxTM);
        if (Owner->PhysicsBodyType == EPhysicsBodyType::Kinematic && RigidBody)
        {
            RigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, Owner->PhysicsBodyType == EPhysicsBodyType::Kinematic);
        }
        PxRigidActor = RigidBody;
    }

    if (!PxRigidActor)
    {
        UE_LOG(ELogLevel::Warning, TEXT("Failed to create PxRigidActor for BodyInstance."));
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
        PxRigidActor->attachShape(*Shape);

        Shape->release();
    }

    for (const FKSphereElem& SphereElem : AggGeom.SphereElems)
    {
        PxSphereGeometry SphereGeom(SphereElem.Radius);
        PxShape* Shape = Physics->createShape(SphereGeom, *Material);
        if (!Shape)
        {
            continue;
        }
        FTransform SphereLocalTM = SphereElem.GetTransform();
        PxTransform SphereLocalPose = ToPxTransform(SphereLocalTM);
        Shape->setLocalPose(SphereLocalPose);
        PxRigidActor->attachShape(*Shape);
        Shape->release();
    }

    for (const FKSphylElem& SphylElem : AggGeom.SphylElems)
    {
        PxCapsuleGeometry CapsuleGeom(SphylElem.Radius, SphylElem.Length * 0.5f);
        PxShape* Shape = Physics->createShape(CapsuleGeom, *Material);
        if (!Shape)
        {
            continue;
        }
        FTransform SphylLocalTM = SphylElem.GetTransform();
        PxTransform SphylLocalPose = ToPxTransform(SphylLocalTM);
        Shape->setLocalPose(SphylLocalPose);
        PxRigidActor->attachShape(*Shape);
        Shape->release();
    }



    if (Owner->PhysicsBodyType == EPhysicsBodyType::Dynamic && RigidBody)
    {
        PxRigidBodyExt::updateMassAndInertia(*RigidBody, BodySetup->MassInKg > 0 ? BodySetup->MassInKg : 1.0f);
    }

    Scene->addActor(*PxRigidActor);
    bInitialized = true;
}

void FBodyInstance::UpdateComponentTransform(class UPrimitiveComponent* Owner, float DeltaTime)
{
    if (!PxRigidActor)
    {
        return;
    }

    const PxTransform PxTM = PxRigidActor->getGlobalPose();
    const FTransform WorldTM = FromPxTransform(PxTM);
    Owner->SetWorldTransform(WorldTM);
    
}

void FBodyInstance::SetBodyTransform(const FTransform& NewTransform, bool bTeleport)
{
    if (!PxRigidActor)
    {
        return;
    }

    const PxTransform PxTM = ToPxTransform(NewTransform);

    if (PxRigidDynamic* Dynamic = PxRigidActor->is<PxRigidDynamic>())
    {
        if (Dynamic->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
        {
            if (bTeleport)
            {
                Dynamic->setGlobalPose(PxTM);
            }
            else
            {
                Dynamic->setKinematicTarget(PxTM);
            }
        }
    }
    else
    {
        PxRigidActor->setGlobalPose(PxTM);
    }
}

FTransform FBodyInstance::GetBodyTransform() const
{
    if (!PxRigidActor)
    {
        return FTransform::Identity;
    }
    const PxTransform PxTM = PxRigidActor->getGlobalPose();
    return FromPxTransform(PxTM);
}
