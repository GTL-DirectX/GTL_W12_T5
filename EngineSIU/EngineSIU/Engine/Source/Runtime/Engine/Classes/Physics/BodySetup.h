#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "AggregateGeom.h"

// Skeletal Mesh 본에 매핑되는 물리 본체 구성에 사용.
// 물리 본체는 물리 시뮬레이션을 위해 사용되는 물리적 본체를 정의.
// 각 본마다 하나의 UBodySetup이 존재.
class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)

public:
    // ECollisionTraceFlag CollisionTraceFlag;

    FName BoneName;

    
    
};

class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
    
public:
    
    struct FKAggregateGeom AggGeom;
    
};
