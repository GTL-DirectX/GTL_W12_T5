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

    // 월드 기준 Transform
    const PxTransform ParentWorldTM = Actor2->getGlobalPose(); // Parent = ConstraintBone2
    const PxTransform ChildWorldTM = Actor1->getGlobalPose();  // Child  = ConstraintBone1

    /*PxTransform LocalPose1 = ToPxTransform(Frame1.Position, Frame1.PriAxis, Frame1.SecAxis);
    PxTransform LocalPose2 = ToPxTransform(Frame2.Position, Frame2.PriAxis, Frame2.SecAxis);*/

    // 상대 Transform 계산 (Local Frame)
    const PxTransform LocalFrameParent = ParentWorldTM.getInverse() * ChildWorldTM;
    const PxTransform LocalFrameChild = PxTransform(PxVec3(0)); // Child 기준은 원점

    PxD6Joint* JointHandle = PxD6JointCreate(*Physics, Actor2, LocalFrameParent, Actor1, LocalFrameChild);
    if (!JointHandle)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create D6 joint."));
        return;
    }

    JointHandle->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, !bDisableCollision);

    JointHandle->setMotion(PxD6Axis::eX, ToPxMotion(XMotion));
    JointHandle->setMotion(PxD6Axis::eY, ToPxMotion(YMotion));
    JointHandle->setMotion(PxD6Axis::eZ, ToPxMotion(ZMotion));

    JointHandle->setMotion(PxD6Axis::eTWIST, ToPxMotion(TwistMotion));
    JointHandle->setMotion(PxD6Axis::eSWING1, ToPxMotion(Swing1Motion));
    JointHandle->setMotion(PxD6Axis::eSWING2, ToPxMotion(Swing2Motion));

    //if (XMotion == ELinearConstraintMotion::Limited ||
    //    YMotion == ELinearConstraintMotion::Limited ||
    //    ZMotion == ELinearConstraintMotion::Limited)
    //{
    //    PxJointLinearLimit LinearLimit(Limits.Linear, PxSpring(0,0)); 
    //    if (XMotion == ELinearConstraintMotion::Limited)
    //    {
 
    //    JointHandle->setLinearLimit(LinearLimit);
    //}

    //if (Swing1Motion == EAngularConstraintMotion::Limited || Swing2Motion == EAngularConstraintMotion::Limited)
    //{
    //    PxJointLimitCone SwingLimit(
    //        FMath::DegreesToRadians(Limits.Swing1Deg),
    //        FMath::DegreesToRadians(Limits.Swing2Deg),
    //        0.01f);
    //    JointHandle->setSwingLimit(SwingLimit);
    //}

    //if (TwistMotion == EAngularConstraintMotion::Limited)
    //{
    //    PxJointAngularLimitPair TwistLimit(
    //        FMath::DegreesToRadians(-Limits.TwistDeg),
    //        FMath::DegreesToRadians(+Limits.TwistDeg),
    //        0.01f);
    //    JointHandle->setTwistLimit(TwistLimit);
    //}

    //// 브레이크
    //if (BreakInfo.bLinearBreakable || BreakInfo.bAngularBreakable)
    //{
    //    JointHandle->setBreakForce(
    //        BreakInfo.bLinearBreakable  ? BreakInfo.LinearThreshold  : PX_MAX_F32,
    //        BreakInfo.bAngularBreakable ? BreakInfo.AngularThreshold : PX_MAX_F32);
    //    JointHandle->setConstraintFlag(PxConstraintFlag::eBROKEN, true);
    //}

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

    if (BreakInfo.bLinearBreakable || BreakInfo.bAngularBreakable)
    {
        JointHandle->setBreakForce(
            BreakInfo.bLinearBreakable ? BreakInfo.LinearThreshold : PX_MAX_F32,
            BreakInfo.bAngularBreakable ? BreakInfo.AngularThreshold : PX_MAX_F32);
        JointHandle->setConstraintFlag(PxConstraintFlag::eBROKEN, true);
    }

}

void FConstraintInstance::TermConstraint()
{
}

