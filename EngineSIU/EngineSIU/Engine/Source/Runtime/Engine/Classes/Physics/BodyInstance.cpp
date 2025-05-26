#include "BodyInstance.h"

#include "BodySetup.h"
#include "PhysXManager.h"
#include "Components/PrimitiveComponent.h"

void FBodyInstance::InitBody(PxScene* Scene, class UPrimitiveComponent* Owner)
{
    if (bInitialized)
        return;

    if (!BodySetup || !Scene)
        return;

    PxPhysics* Physics = FPhysXManager::Get().GetPhysics();
    if (!Physics)
        return;
    
    
    
}
