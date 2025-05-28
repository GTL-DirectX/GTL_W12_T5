#include "ConstraintInstance.h"

#include "BodyInstance.h"
#include "PhysicsAssetUtils.h"
#include "PhysXManager.h"

inline PxD6Motion::Enum ToPxMotion(ELinearConstraintMotion Motion)
{
    switch (Motion)
    {
    case ELinearConstraintMotion::Locked: return PxD6Motion::eLOCKED;
    case ELinearConstraintMotion::Limited: return PxD6Motion::eLIMITED;
    case ELinearConstraintMotion::Free: return PxD6Motion::eFREE;
    default: return PxD6Motion::eLOCKED;
    }
}

inline PxD6Motion::Enum ToPxMotion(EAngularConstraintMotion Motion)
{
    switch (Motion)
    {
    case EAngularConstraintMotion::Locked: return PxD6Motion::eLOCKED;
    case EAngularConstraintMotion::Limited: return PxD6Motion::eLIMITED;
    case EAngularConstraintMotion::Free: return PxD6Motion::eFREE;
    default: return PxD6Motion::eLOCKED;
    }
}

void FConstraintInstance::InitConstraint(FBodyInstance* Body1, FBodyInstance* Body2)
{
    // TODO: 내용 채우기.
    if (!Body1 || !Body2)
    {
        return;
    }

    TermConstraint();

    PxPhysics* Physics = FPhysXManager::Get().GetPhysics();
    if (!Physics)
    {
        return;
    }

    PxRigidActor* Actor1 = Body1->PxRigidActor;
    PxRigidActor* Actor2 = Body2->PxRigidActor;
    if (!Actor1 || !Actor2)
    {
        return;
    }

    PxTransform LocalPose1 = ToPxTransform(Frame1.Position, Frame1.PriAxis, Frame1.SecAxis);
    PxTransform LocalPose2 = ToPxTransform(Frame2.Position, Frame2.PriAxis, Frame2.SecAxis);

    PxD6Joint* JointHandle = PxD6JointCreate(*Physics, Actor2, LocalPose2, Actor1, LocalPose1);
    if (!JointHandle)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create D6 joint."));
        return;
    }

    JointHandle->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, !bDisableCollision);

    JointHandle->setMotion(PxD6Axis::eX, ToPxMotion(Swing1Motion));
    JointHandle->setMotion(PxD6Axis::eY, ToPxMotion(Swing2Motion));
    JointHandle->setMotion(PxD6Axis::eZ, ToPxMotion(TwistMotion));

    // 선형 제한 설정 (만약 Limited 모드일 경우)
    // PxJointLinearLimit은 대칭적인 제한 (예: -Limit to +Limit)
    // PxJointLinearLimitPair는 비대칭적인 제한 (예: Lower to Upper)
    // 여기서는 Limits.Linear를 대칭적인 제한 값으로 사용합니다.
    if (XMotion == ELinearConstraintMotion::Limited ||
        YMotion == ELinearConstraintMotion::Limited ||
        ZMotion == ELinearConstraintMotion::Limited)
    {
        // 모든 선형 축에 대해 동일한 제한을 적용할지,
        // 아니면 각 축별로 다른 제한 값을 가질지는 FConstraintLimit 구조체 설계에 따라 달라집니다.
        // 현재 Limits.Linear는 단일 float이므로, 모든 제한된 선형 축에 이 값을 적용한다고 가정합니다.
        // 실제로는 각 축(X, Y, Z)에 대해 별도의 Limit 값을 FConstraintLimit에 두는 것이 더 유연합니다.
        // 예: Limits.LinearX, Limits.LinearY, Limits.LinearZ
        // 여기서는 Limits.Linear를 모든 제한된 선형 축의 대칭적 한계로 사용합니다.
        PxJointLinearLimit LinearLimit(Limits.Linear, PxSpring(0,0)); // 스프링은 사용하지 않음 (필요시 설정)
                                                                    // PxTolerancesScale scale; // PhysX 초기화 시 사용된 스케일
                                                                    // PxJointLinearLimit LinearLimit(scale, Limits.Linear, PxSpring(0,0)); // PhysX 4.x+
        if (XMotion == ELinearConstraintMotion::Limited)
        {
            // PxD6Joint는 각 축에 대해 개별적인 LinearLimit을 설정하는 API가 직접적으로는 없습니다.
            // 대신, PxD6Drive를 사용하거나, 혹은 PxD6Motion::eLIMITED 상태에서
            // 조인트 프레임 자체를 잘 설정하여 원하는 효과를 내야 합니다.
            // 가장 일반적인 방법은 PxJointLinearLimit을 설정하고,
            // 이것이 조인트 프레임의 각 축에 어떻게 적용될지 이해하는 것입니다.
            // PxD6Joint의 경우, setLinearLimit은 모든 선형 자유도에 대한 "거리" 제한처럼 동작할 수 있습니다.
            // 좀 더 정교한 제어를 위해서는 각 축에 대한 드라이브를 설정하거나,
            // 혹은 여러 개의 간단한 조인트를 조합해야 할 수 있습니다.
            // 여기서는 setLinearLimit이 모든 제한된 선형 축에 적용된다고 가정합니다.
            // (주의: PxD6Joint의 setLinearLimit은 단일 값만 받으며, 이는 모든 선형 자유도에 대한 거리 제한입니다.
            //  각 축별로 다른 선형 제한을 걸려면, 조인트 프레임을 회전시키거나,
            //  혹은 PxD6Drive를 사용하여 특정 축 방향으로의 움직임을 제어해야 합니다.)
            //  가장 간단한 해석은, 만약 어떤 선형 축이 Limited이면, 원점으로부터 Limits.Linear 거리 내에서만 움직일 수 있다는 것입니다.
        }
        // Y, Z에 대해서도 유사하게 처리 (단, setLinearLimit은 한 번만 호출)
        JointHandle->setLinearLimit(LinearLimit);
    }

    if (Swing1Motion == EAngularConstraintMotion::Limited || Swing2Motion == EAngularConstraintMotion::Limited)
    {
        PxJointLimitCone SwingLimit(
            FMath::DegreesToRadians(Limits.Swing1Deg),
            FMath::DegreesToRadians(Limits.Swing2Deg),
            0.01f);
        JointHandle->setSwingLimit(SwingLimit);
    }

    if (TwistMotion == EAngularConstraintMotion::Limited)
    {
        PxJointAngularLimitPair TwistLimit(
            FMath::DegreesToRadians(-Limits.TwistDeg),
            FMath::DegreesToRadians(+Limits.TwistDeg),
            0.01f);
        JointHandle->setTwistLimit(TwistLimit);
    }

    // 브레이크
    if (BreakInfo.bLinearBreakable || BreakInfo.bAngularBreakable)
    {
        JointHandle->setBreakForce(
            BreakInfo.bLinearBreakable  ? BreakInfo.LinearThreshold  : PX_MAX_F32,
            BreakInfo.bAngularBreakable ? BreakInfo.AngularThreshold : PX_MAX_F32);
        JointHandle->setConstraintFlag(PxConstraintFlag::eBROKEN, true);
    }

}

void FConstraintInstance::TermConstraint()
{
}

