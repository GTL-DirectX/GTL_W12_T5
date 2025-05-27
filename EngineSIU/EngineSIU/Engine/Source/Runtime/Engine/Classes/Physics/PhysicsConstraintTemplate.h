#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "ConstraintInstance.h"

class UPhysicsConstraintTemplate : public UObject
{
    DECLARE_CLASS(UPhysicsConstraintTemplate, UObject)
    
public:
    // 실제 제약 값.
    FConstraintInstance DefaultInstance;
    
    UPhysicsConstraintTemplate() = default;


    
    virtual void Serialize(FArchive& Ar) override;
    
    

};
