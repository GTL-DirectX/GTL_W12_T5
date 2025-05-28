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
    float, X, = 1.0f;
    )

    UPROPERTY(
    EditAnywhere,
    float, Y, = 1.0f;
    )

    UPROPERTY(
    EditAnywhere,
    float, Z, = 1.0f;
    )
    



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


    void Serialize(FArchive& Ar) override
    {
        FKShapeElem::Serialize(Ar);
        Ar << Center;
        Ar << Rotation;
        Ar << X;
        Ar << Y;
        Ar << Z;
    }
    friend FArchive& operator<<(FArchive& Ar, FKBoxElem& Elem);
};

inline FArchive& operator<<(FArchive& Ar, FKBoxElem& Elem)
{
    Ar.Serialize(Elem);
    return Ar;
}
