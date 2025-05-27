#include "SimulationViewerWorld.h"

USimulationViewerWorld* USimulationViewerWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{
    USimulationViewerWorld* NewWorld = FObjectFactory::ConstructObject<USimulationViewerWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();
    NewWorld->SelectBoneIndex = 0;



    
    
    return NewWorld;
}

void USimulationViewerWorld::Tick(float DeltaTime)
{
    UWorld::Tick(DeltaTime);
}
