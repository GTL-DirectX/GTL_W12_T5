#pragma once

#include "ShapeElem.h"

// Capsule shapde used for collsion. Z axis is capsule axis.
struct FKSphylElem : public FKShapeElem
{
    DECLARE_STRUCT(FKSphylElem, FKShapeElem)

        UPROPERTY(
    EditAnywhere,
    FVector, Center, = FVector::ZeroVector;
    )   

        UPROPERTY(
    EditAnywhere,
    FRotator, Rotation, = FRotator::ZeroRotator;
    )

    UPROPERTY(
    EditAnywhere,
    float, Radius, = 0.5f;
    )
    
    UPROPERTY(
    EditAnywhere,
    float, Length, = 0.5f;
    )   

    FKSphylElem()
        : FKShapeElem(EAggCollisionShape::Sphyl)
        , Center(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , Radius(0.0f)
        , Length(0.0f)
    {
    }

    FKSphylElem(float InRadius, float InLength)
        : FKShapeElem(EAggCollisionShape::Sphyl)
        , Center(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , Radius(InRadius)
        , Length(InLength)
    {
    }

    FTransform GetTransform() const
    {
        return FTransform(Rotation, Center);
    }

    void SetTransform(const FTransform& InTransform)
    {
        Center = InTransform.GetTranslation();
        Rotation = InTransform.GetRotation().Rotator();
    }

    
    void Serialize(FArchive& Ar) override
    {
        FKShapeElem::Serialize(Ar);
    }
    friend FArchive& operator<<(FArchive& Ar, FKSphylElem& Elem);
};

inline FArchive& operator<<(FArchive& Ar, FKSphylElem& Elem)
{
    Ar.Serialize(Elem);
    return Ar;
}
