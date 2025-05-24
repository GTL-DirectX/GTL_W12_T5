#include "PhysicsAssetUtils.h"


class UBodySetupCore : public UObject
{
    FName BoneName;
};

class UBodySetup : public UBodySetupCore
{
    // DisplayName = Primitives
    struct FKAggregateGeom AggGeom;
};

class UPhysicsAsset : public UObject
{
    TArray<UBodySetup*> BodySetup;
};
