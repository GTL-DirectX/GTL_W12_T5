#include "PhysicsViewerWorld.h"

UPhysicsViewerWorld* UPhysicsViewerWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{    UPhysicsViewerWorld* NewWorld = FObjectFactory::ConstructObject<UPhysicsViewerWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();
    NewWorld->SelectBoneIndex = 0;



    
    
    return NewWorld;
}

void UPhysicsViewerWorld::Tick(float DeltaTime)
{
    UWorld::Tick(DeltaTime);
}
