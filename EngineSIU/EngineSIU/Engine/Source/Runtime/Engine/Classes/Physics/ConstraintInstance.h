#pragma once

#include "PhysicalMaterial.h"
#include "UObject/ObjectMacros.h"

struct FBodyInstance;

namespace physx
{
    class PxScene;
}

enum class ELinearConstraintMotion : uint8 { Locked, Limited, Free };
enum class EAngularConstraintMotion : uint8 { Locked, Limited, Free };

struct FConstraintFrame
{
    FVector Position = FVector::ZeroVector;
    FVector PriAxis = FVector::ForwardVector;
    FVector SecAxis = FVector::RightVector;
};

struct FConstraintLimit
{
    float Linear = 50.0f; // cm.
    float Swing1Deg = 45.0f; // degrees.
    float Swing2Deg = 45.0f;
    float TwistDeg = 45.0f;
};

struct FConstraintBreakData
{
    bool bLinearBreakable = false;
    float LinearThreshold = 50000.0f; // N.
    bool bAngularBreakable = false;
    float AngularThreshold = 50000.0f; // N * cm.
};

struct FConstraintInstanceBase
{
    DECLARE_STRUCT(FConstraintInstanceBase)

    int32 ConstraintIndex = INDEX_NONE;

    // Constraint가 포함된 Scene.
    physx::PxScene* PxScene = nullptr;

    physx::PxScene* GetPxScene() const { return PxScene; }
    
};

struct FConstraintInstance : public FConstraintInstanceBase
{
    DECLARE_STRUCT(FConstraintInstance, FConstraintInstanceBase)
    
public:
    // 식별용 변수.
    UPROPERTY(EditAnywhere, FName, JointName, = NAME_None)
    // Constraint가 연결될 Bone 이름. PhysicsAsset에서 Child Bone에 적용.
    UPROPERTY(EditAnywhere, FName, ConstraintBone1, = NAME_None)
    // Constraint가 연결될 Bone 이름. PhysicsAsset에서 Parent Bone에 적용.
    UPROPERTY(EditAnywhere, FName, ConstraintBone2, = NAME_None)

    // 로컬 프레임.
    // Constraint가 연결된 Bone의 로컬 좌표계에서의 제약 기준 위치와 회전.
    FConstraintFrame Frame1; // Child 기준
    FConstraintFrame Frame2; // Parent 기준
    
    // 제약 모션 설정.
    ELinearConstraintMotion   XMotion = ELinearConstraintMotion::Locked;
    ELinearConstraintMotion   YMotion = ELinearConstraintMotion::Locked;
    ELinearConstraintMotion   ZMotion = ELinearConstraintMotion::Locked;

    EAngularConstraintMotion  Swing1Motion = EAngularConstraintMotion::Locked;
    EAngularConstraintMotion  Swing2Motion = EAngularConstraintMotion::Locked;
    EAngularConstraintMotion  TwistMotion  = EAngularConstraintMotion::Locked;

    FConstraintLimit          Limits;
    bool                      bDisableCollision = false;

    /*----------- 브레이크 ------------*/
    FConstraintBreakData      BreakInfo;

    /*=========== API ===========*/
    void InitConstraint(FBodyInstance* Body1, FBodyInstance* Body2);
    void TermConstraint();
    
    
};
