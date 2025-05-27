#pragma once

#include "Container/Array.h"
#include "PropertyEditor/ShowFlags.h"
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

    FKShapeElem()
    : RestOffset(0.f)
    , bIsGenerated(false)
    , ShapeType(EAggCollisionShape::Unknown)
    , bContributeToMass(true)
    , CollisionEnabled(ECollisionEnabled::QueryAndPhysics)
    {}

    FKShapeElem(EAggCollisionShape::Type InShapeType)
    : RestOffset(0.f)
    , bIsGenerated(false)
    , ShapeType(InShapeType)
    , bContributeToMass(true)
    , CollisionEnabled(ECollisionEnabled::QueryAndPhysics)
    {}

    FKShapeElem(const FKShapeElem& Other)
    : RestOffset(Other.RestOffset)
    , bIsGenerated(Other.bIsGenerated)
    , Name(Other.Name)
    , ShapeType(Other.ShapeType)
    , bContributeToMass(Other.bContributeToMass)
    , CollisionEnabled(Other.CollisionEnabled)
    {}

    virtual ~FKShapeElem() {}

    const FKShapeElem& operator=(const FKShapeElem& Other)
    {
        CloneElem(Other);
        return *this;
    }

    template <typename T>
    T* GetShapeCheck()
    {
        if (T::StaticShapeType == ShapeType)
        {
            return static_cast<T*>(this);
        }
        return nullptr;
    }


    const FName& GetName() const { return Name; }
    void SetName(const FName& InName) { Name = InName; }
    EAggCollisionShape::Type GetShapeType() const { return ShapeType; }
    bool GetContributeToMass() const { return bContributeToMass; }
    void SetContributeToMass(bool bInContributeToMass) { bContributeToMass = bInContributeToMass; }
    void SetCollionEnabled(ECollisionEnabled::Type InCollisionEnabled) { CollisionEnabled = InCollisionEnabled; }
    ECollisionEnabled::Type GetCollisionEnabled() const { return CollisionEnabled; }

    /* 
        접촉 계산 시 오프셋 거리.
        충돌 검출 시 두 충돌 사이 약간의 여유 간격을 두어 충돌 감지를 부드럽게 만들기 위한 변수.
     */
    UPROPERTY(EditAnywhere, float, RestOffset, = 0.0f)

protected:
    void CloneElem(const FKShapeElem& Other);

private:
    // 사용자 지정 이름.
    UPROPERTY(EditAnywhere, FName, Name, = NAME_None)
    
    EAggCollisionShape::Type ShapeType = EAggCollisionShape::Sphere;

    // 질량 계산에 기여하는지 여부.
    UPROPERTY(EditAnywhere, bool, bContributeToMass, = true);

    ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    uint8 bIsGenerated = false;


public:
    virtual void Serialize(FArchive& Ar)
    {
        int ShapeTypeInt = static_cast<int>(ShapeType);
        int CollisionEnabledInt = static_cast<int>(CollisionEnabled);
        Ar << RestOffset 
                << Name 
                << ShapeTypeInt
                << bContributeToMass 
                << CollisionEnabledInt
                << bIsGenerated;
        ShapeType = static_cast<EAggCollisionShape::Type>(ShapeTypeInt);
        CollisionEnabled = static_cast<ECollisionEnabled::Type>(CollisionEnabledInt);
    }



   friend FArchive& operator<<(FArchive& Ar, FKShapeElem& V);
    
};

inline FArchive& operator<<(FArchive& Ar, FKShapeElem& V)
{
    V.Serialize(Ar);
    return Ar;
}



