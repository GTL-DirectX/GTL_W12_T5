#pragma once
#include "World.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


//일단 안씀
class USimulationViewerWorld : public UWorld
{
    DECLARE_CLASS(USimulationViewerWorld, UWorld)
    
public:
    USimulationViewerWorld() = default;
    
    static USimulationViewerWorld* CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName = "DefaultWorld");

    void SetSkeletalMeshComponent(USkeletalMeshComponent* Component)
    {
        SkeletalMeshComponent = Component;
    }
    USkeletalMeshComponent* GetSkeletalMeshComponent()
    {
        return SkeletalMeshComponent;
    }

    int32 SelectBoneIndex = 0;

    void Tick(float DeltaTime) override;

private:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
};
