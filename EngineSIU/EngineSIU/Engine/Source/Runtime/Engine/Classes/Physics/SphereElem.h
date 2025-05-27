#pragma once

#include "ShapeElem.h"

struct FKSphereElem : public FKShapeElem
{
    DECLARE_STRUCT(FKSphereElem, FKShapeElem)

    UPROPERTY(
    EditAnywhere,
    FVector, Center, = FVector::ZeroVector;
    )   

    UPROPERTY(
    EditAnywhere,
    float, Radius, = 0.5f;
    )   
    
    FTransform GetTransform() const
    {
        return FTransform(Center);
    }

    void SetTransform(const FTransform& NewTransform)
    {
        Center = NewTransform.GetTranslation();
    }

    FKSphereElem()
        : FKShapeElem(EAggCollisionShape::Type::Sphere)
        , Center(FVector::ZeroVector)
        , Radius(0.5f)
    {
    }

    FKSphereElem(float r)
        : FKShapeElem(EAggCollisionShape::Type::Box)
        , Center(FVector::ZeroVector)
        , Radius(r)
    {
    }

    /*FVector GetScaledVolume(const FVector& Scale) const
    {
        return 1.3333f * PI * FMath::Pow(Radius * Scale.GetAbs(), 3);
    }*/


    void Serialize(FArchive& Ar) override
    {
        FKShapeElem::Serialize(Ar);
    }
    friend FArchive& operator<<(FArchive& Ar, FKSphereElem& Elem);
};

inline FArchive& operator<<(FArchive& Ar, FKSphereElem& Elem)
{
    Ar.Serialize(Elem);
    return Ar;
}
