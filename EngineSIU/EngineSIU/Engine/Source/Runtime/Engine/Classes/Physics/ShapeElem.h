#pragma once

#include "Container/Array.h"
#include "UObject/ObjectMacros.h"

// TODO: EngineType으로 옮겨져야 하는 Enum.
namespace ECollisionEnabled 
{ 
enum Type : int
{ 
    /** Will not create any representation in the physics engine. Cannot be used for spatial queries (raycasts, sweeps, overlaps) or simulation (rigid body, constraints). Best performance possible (especially for moving objects) */
    NoCollision, 
    /** Only used for spatial queries (raycasts, sweeps, and overlaps). Cannot be used for simulation (rigid body, constraints). Useful for character movement and things that do not need physical simulation. Performance gains by keeping data out of simulation tree. */
    QueryOnly,
    /** Only used only for physics simulation (rigid body, constraints). Cannot be used for spatial queries (raycasts, sweeps, overlaps). Useful for jiggly bits on characters that do not need per bone detection. Performance gains by keeping data out of query tree */
    PhysicsOnly,
    /** Can be used for both spatial queries (raycasts, sweeps, overlaps) and simulation (rigid body, constraints). */
    QueryAndPhysics,
    /** Only used for probing the physics simulation (rigid body, constraints). Cannot be used for spatial queries (raycasts,
    sweeps, overlaps). Useful for when you want to detect potential physics interactions and pass contact data to hit callbacks
    or contact modification, but don't want to physically react to these contacts. */
    ProbeOnly,
    /** Can be used for both spatial queries (raycasts, sweeps, overlaps) and probing the physics simulation (rigid body,
    constraints). Will not allow for actual physics interaction, but will generate contact data, trigger hit callbacks, and
    contacts will appear in contact modification. */
    QueryAndProbe,
}; 
}


namespace EAggCollisionShape
{
    enum Type
    {
        Sphere,
        Box,
        Sphyl,
        Convex,
        TaperedCapsule,
        LevelSet,
        SkinnedLevelSet,

        Unknown
    };
}

struct FKShapeElem
{
    DECLARE_STRUCT(FKShapeElem)
    
public:

    /* 
        접촉 계산 시 오프셋 거리.
        충돌 검출 시 두 충돌 사이 약간의 여유 간격을 두어 충돌 감지를 부드럽게 만들기 위한 변수.
     */
    UPROPERTY(EditAnywhere, float, RestOffset, = 0.0f)

private:
    // 사용자 지정 이름.
    UPROPERTY(EditAnywhere, FName, Name, = NAME_None)
    
    EAggCollisionShape::Type ShapeType = EAggCollisionShape::Sphere;

    // 질량 계산에 기여하는지 여부.
    UPROPERTY(EditAnywhere, bool, bContributeToMass, = true);

    ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    

    void Serialize(FArchive& Ar)
    {
        int ShapeTypeInt = static_cast<int>(ShapeType);
        int CollisionEnabledInt = static_cast<int>(CollisionEnabled);
        Ar << RestOffset 
                << Name 
                << ShapeTypeInt
                << bContributeToMass 
                << CollisionEnabledInt;
        ShapeType = static_cast<EAggCollisionShape::Type>(ShapeTypeInt);
        CollisionEnabled = static_cast<ECollisionEnabled::Type>(CollisionEnabledInt);
   }


    friend FArchive& operator<<(FArchive& Ar, FKShapeElem& V);
    
};

inline FArchive& operator<<(FArchive& Ar, FKShapeElem& V)
{
    int ShapeTypeInt = static_cast<int>(V.ShapeType);
    int CollisionEnabledInt = static_cast<int>(V.CollisionEnabled);

    Ar << V.RestOffset 
              << V.Name 
              << ShapeTypeInt
              << V.bContributeToMass 
              << CollisionEnabledInt;

    
    V.ShapeType = static_cast<EAggCollisionShape::Type>(ShapeTypeInt);
    V.CollisionEnabled = static_cast<ECollisionEnabled::Type>(CollisionEnabledInt);
    return Ar;

    
}



