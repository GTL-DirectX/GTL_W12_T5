#pragma once

#include "ConstraintInstance.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UPhysicsConstraintTemplate : public UObject
{
    DECLARE_CLASS(UPhysicsConstraintTemplate, UObject)
    
public:
    // 실제 제약 값.
    FConstraintInstance DefaultInstance;
    
    UPhysicsConstraintTemplate() = default;


};
