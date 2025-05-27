#pragma once

#include "ShapeElem.h"

// Capsule shapde used for collsion. Z axis is capsule axis.
struct FKSphylElem : public FKShapeElem
{
    DECLARE_STRUCT(FKSphylElem, FKShapeElem)

    FVector Center = FVector::ZeroVector; // 중심 위치
    FRotator Rotation = FRotator::ZeroRotator; // 회전
    float Radius = 0.0f; // 반지름
    float Length = 0.0f; // 길이

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

    
};
