#pragma once

#include <PxPhysicsAPI.h>
#include "Math/Transform.h"
#include "Math/Matrix.h"

// PxVec3 = FVector;
// PxQuat = FQuat;
// PxTransform = FTransform;
// PxRaycastHit = FHitResult;
// PxOverlapHit = FOverlapResult;
// PxSweepHit = FSweepResult;
// PxFilterData = FMaskFilter;
// PxMaterial = UPhysicalMaterial;
// PxShape = FBodyInstance;
// PxRigidActor = FBodyInst;ance
// PxRigidDynamic = FBodyInstance;
// PxRigidStatic = FBodyInstance;
// PxJoint = FConstraintInstance;
// PxScene = UWorld->GetPhysicsScene();

using namespace physx;

inline PxVec3 ToPxVec3(const FVector& Vec)
{
    return PxVec3(Vec.X, Vec.Y, Vec.Z);
}

inline PxQuat ToPxQuat(const FQuat& Quat)
{
    return PxQuat(Quat.X, Quat.Y, Quat.Z, Quat.W);
}

inline PxTransform ToPxTransform(const FTransform& Transform)
{
    return PxTransform(
        ToPxVec3(Transform.GetTranslation()),
        ToPxQuat(Transform.GetRotation())
    );
}

// FMatrix 기반 Z-Up, Left-Hand 좌표계 변환 함수
// 축 관련 문제 생길 경우 이 함수부터 봐야함.
inline PxTransform ToPxTransform(const FVector& Position, const FVector& PriAxis, const FVector& SecAxis)
{
    // 1. 기준 축 정규화
    const FVector X = PriAxis.GetSafeNormal();    // 기준축
    const FVector Y = SecAxis.GetSafeNormal();    // 보조축
    const FVector Z = X ^ Y;                      // 왼손 좌표계: Z = X × Y
    const FVector YFix = Z ^ X;                   // Y 보정으로 직교성 보장

    // 2. 행렬 구성 (당신의 엔진은 행-기반)
    FMatrix RotMatrix;
    RotMatrix.SetAxis(0, X);     // Row 0 = X축
    RotMatrix.SetAxis(1, YFix);  // Row 1 = 보정된 Y축
    RotMatrix.SetAxis(2, Z);     // Row 2 = Z축
    RotMatrix.SetAxis(3, FVector::ZeroVector); // Row 3 = 변환 없음

    // 3. 행렬 → 쿼터니언
    const FQuat RotQuat = RotMatrix.ToQuat();

    return PxTransform(ToPxVec3(Position), ToPxQuat(RotQuat));
}

