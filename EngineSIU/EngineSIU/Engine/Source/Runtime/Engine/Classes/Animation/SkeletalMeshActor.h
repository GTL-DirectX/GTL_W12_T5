#pragma once
#include "GameFramework/Actor.h"

class USkeletalMeshComponent;

class ASkeletalMeshActor : public AActor
{
    DECLARE_CLASS(ASkeletalMeshActor, AActor)

public:
    ASkeletalMeshActor();
    virtual void PostSpawnInitialize() override;
    virtual ~ASkeletalMeshActor() override = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

private:
    UPROPERTY(EditAnywhere, USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr)
    
};
