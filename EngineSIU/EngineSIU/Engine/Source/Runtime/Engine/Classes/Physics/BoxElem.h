#pragma once

#include "ShapeElem.h"

struct FBox
{
    FVector Min;
    FVector Max;
};

struct FKBoxElem : public FKShapeElem
{
    DECLARE_STRUCT(FKBoxElem, FKShapeElem)

    // Position of Box's origin.
    FVector Center;

    // Rotation of the Box.
    FRotator Rotation;

    // Extent of the Box along the X-axis.
    float X;
    // Extent of the Box along the Y-axis.
    float Y;
    // Extent of the Box along the Z-axis.
    float Z;


    FKBoxElem()
    : FKShapeElem(EAggCollisionShape::Type::Box)
    , Center( FVector::ZeroVector )
    , Rotation( FRotator::ZeroRotator )
    , X(1.0f), Y(1.0f), Z(1.0f)
    {
    }

    FKBoxElem(float s)
    : FKShapeElem(EAggCollisionShape::Type::Box)
    , Center( FVector::ZeroVector )
    , Rotation( FRotator::ZeroRotator )
    , X(s), Y(s), Z(s)
    {
    }

    FKBoxElem(float InX, float InY, float InZ)
    : FKShapeElem(EAggCollisionShape::Type::Box)
    , Center( FVector::ZeroVector )
    , Rotation( FRotator::ZeroRotator )
    , X(InX), Y(InY), Z(InZ)
    {
    }

    FTransform GetTransform() const
    {
        return FTransform(Rotation, Center);
    }

    void SetTransform(const FTransform& InTransform)
    {
        Rotation = InTransform.GetRotation().Rotator();
        Center = InTransform.GetTranslation();
    }

    float GetScaledVolume(const FVector& Scale3D) const { return FMath::Abs(Scale3D.X * Scale3D.Y * Scale3D.Z * X * Y * Z); }
    
};
