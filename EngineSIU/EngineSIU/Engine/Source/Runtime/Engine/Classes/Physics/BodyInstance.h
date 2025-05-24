#pragma once

#include "Hal/PlatformType.h"
#include "UObject/ObjectMacros.h"

struct FBodyInstanceCore
{
    DECLARE_STRUCT(FBodyInstanceCore)
    
    UPROPERTY(EditAnywhere, bool, bSimulatePhysics, = true)


    
};


struct FBodyInstance : public FBodyInstanceCore
{
    DECLARE_STRUCT(FBodyInstance, FBodyInstanceCore)

    int32 InstanceBodyIndex;
    
    
};
