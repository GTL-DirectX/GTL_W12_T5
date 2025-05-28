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
    DECLARE_STRUCT(FConstraintFrame)
    
    UPROPERTY(EditAnywhere, FVector, Position, = FVector::ZeroVector)
    UPROPERTY(EditAnywhere, FVector, PriAxis, = FVector::ForwardVector)
    UPROPERTY(EditAnywhere, FVector, SecAxis, = FVector::RightVector)
    
};

struct FConstraintLimit
{
    DECLARE_STRUCT(FConstraintLimit)

    UPROPERTY(EditAnywhere, float, Linear, = 50.0f)
    UPROPERTY(EditAnywhere, float, Swing1Deg, = 45.0f)
    UPROPERTY(EditAnywhere, float, Swing2Deg, = 45.0f)
    UPROPERTY(EditAnywhere, float, TwistDeg, = 45.0f)
};

struct FConstraintBreakData
{
    DECLARE_STRUCT(FConstraintBreakData)

    UPROPERTY(EditAnywhere, bool, bLinearBreakable, = false)
    UPROPERTY(EditAnywhere, float, LinearThreshold, = 50000.0f) // N.
    UPROPERTY(EditAnywhere, bool, bAngularBreakable, = false)
    UPROPERTY(EditAnywhere, float, AngularThreshold, = 50000.0f) // N * cm.
};

struct FConstraintInstanceBase
{
    DECLARE_STRUCT(FConstraintInstanceBase)

    UPROPERTY(EditAnywhere, FName, ConstraintIndex, = NAME_None)

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
    UPROPERTY(
        EditAnywhere,
        FConstraintFrame, Frame1, {};
    )
    UPROPERTY(
        EditAnywhere,
        FConstraintFrame, Frame2, {};
    )

    UPROPERTY(
        EditAnywhere,
        ELinearConstraintMotion, XMotion, = ELinearConstraintMotion::Locked;
    )
    UPROPERTY(
        EditAnywhere,
        ELinearConstraintMotion, YMotion, = ELinearConstraintMotion::Locked;
    )
    UPROPERTY(
        EditAnywhere,
        ELinearConstraintMotion, ZMotion, = ELinearConstraintMotion::Locked;
    )

    
    UPROPERTY(
        EditAnywhere,
        EAngularConstraintMotion, Swing1Motion, = EAngularConstraintMotion::Locked;
    )
    UPROPERTY(
        EditAnywhere,
        EAngularConstraintMotion, Swing2Motion, = EAngularConstraintMotion::Locked;
    )
    UPROPERTY(
        EditAnywhere,
        EAngularConstraintMotion, TwistMotion, = EAngularConstraintMotion::Locked;
    )



    UPROPERTY(
        EditAnywhere,
        FConstraintLimit, Limits, {};
    )

    UPROPERTY(
        EditAnywhere,
        bool, bDisableCollision, = false;
    )

    /*----------- 브레이크 ------------*/
    UPROPERTY(
        EditAnywhere,
        FConstraintBreakData, BreakInfo, {};
    )

    /*=========== API ===========*/
    void InitConstraint(FBodyInstance* Body1, FBodyInstance* Body2);
    void TermConstraint();


    friend FArchive& operator<<(FArchive& Ar, FConstraintInstance& Value);
    
};


inline FArchive& operator<<(FArchive& Ar, FConstraintInstance& Value)
{
    int XMotionint  = static_cast<int>(Value.XMotion);
    int YMotionint  = static_cast<int>(Value.YMotion);
    int ZMotionint  = static_cast<int>(Value.ZMotion);

    int Swing1Motion = static_cast<int>(Value.Swing1Motion);
    int Swing2Motion = static_cast<int>(Value.Swing2Motion);
    int TwistMotion  = static_cast<int>(Value.TwistMotion);
    
    
    Ar  << Value.JointName
        << Value.ConstraintBone1
        << Value.ConstraintBone2
        << Value.ConstraintIndex
        << Value.Frame1.Position
        << Value.Frame1.PriAxis
        << Value.Frame1.SecAxis
        << Value.Frame2.Position
        << Value.Frame2.PriAxis
        << Value.Frame2.SecAxis
        << XMotionint
        << YMotionint
        << ZMotionint
        << Swing1Motion
        << Swing2Motion
        << TwistMotion
        << Value.Limits.Linear
        << Value.Limits.Swing1Deg
        << Value.Limits.Swing2Deg
        << Value.Limits.TwistDeg
        << Value.bDisableCollision
        << Value.BreakInfo.bLinearBreakable
        << Value.BreakInfo.LinearThreshold
        << Value.BreakInfo.bAngularBreakable
        << Value.BreakInfo.AngularThreshold;

    Value.XMotion = static_cast<ELinearConstraintMotion>(XMotionint);
    Value.YMotion = static_cast<ELinearConstraintMotion>(YMotionint);
    Value.ZMotion = static_cast<ELinearConstraintMotion>(ZMotionint);

    Value.Swing1Motion = static_cast<EAngularConstraintMotion>(Swing1Motion);
    Value.Swing2Motion = static_cast<EAngularConstraintMotion>(Swing2Motion);
    Value.TwistMotion = static_cast<EAngularConstraintMotion>(TwistMotion);

    return Ar;
    
}
