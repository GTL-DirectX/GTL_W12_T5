#pragma once

#include "BodySetup.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Container/Array.h"

class UBodySetup;
class UPhysicsConstraintTemplate;
struct FBodyInstance;
struct FConstraintInstance;

class USkeletalBodySetup : public UBodySetup
{
    DECLARE_CLASS(USkeletalBodySetup, UBodySetup)

public:
    USkeletalBodySetup() = default;

    
    
    virtual void Serialize(FArchive& Ar) override
    {
        Super::Serialize(Ar);
    }
};

class UPhysicsAsset : public UObject
{
    DECLARE_CLASS(UPhysicsAsset, UObject)
    
public:
    UPhysicsAsset();

    void UpdateBodySetupIndexMap();


public:
    TArray<USkeletalBodySetup*> SkeletalBodySetups;
    TArray<UPhysicsConstraintTemplate*> ConstraintTemplates;

    /** This caches the BodySetup Index by BodyName to speed up FindBodyIndex */
    TMap<FName, int32> BodySetupIndexMap;

    virtual void SerializeAsset(FArchive& Ar) override;

    void CreatePhysicsInstance(class UWorld* World, class USkeletalMeshComponent* SkeletalMeshComponent, TArray<FBodyInstance*>& OutBodies, TArray<FConstraintInstance*>& OutConstraints);

};
