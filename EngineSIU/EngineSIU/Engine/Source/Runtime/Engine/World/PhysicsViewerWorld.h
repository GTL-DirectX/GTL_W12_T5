#pragma once

#include "World.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"



class UPhysicsViewerWorld : public UWorld
{
    DECLARE_CLASS(UPhysicsViewerWorld, UWorld)
    
public:
    UPhysicsViewerWorld() = default;
    
    static UPhysicsViewerWorld* CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName = "DefaultWorld");

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
