#include "PhysicsAsset.h"

#include "PhysicsConstraintTemplate.h"
#include "UObject/ObjectFactory.h"
#include "BodyInstance.h"
#include "World/World.h"

#include "Engine/SkeletalMesh.h"

UPhysicsAsset::UPhysicsAsset()
{
}

void UPhysicsAsset::UpdateBodySetupIndexMap()
{
   // update BodySetupIndexMap
   BodySetupIndexMap.Empty();
   for(int32 i=0; i<SkeletalBodySetups.Num(); i++)
   {
       if ((SkeletalBodySetups[i]))
       {
           BodySetupIndexMap.Add(SkeletalBodySetups[i]->BoneName, i);
       }
   }
}

void UPhysicsAsset::SerializeAsset(FArchive& Ar)
{
    Super::SerializeAsset(Ar);
    {
        // 배열 크기 직렬화
        int ArraySize = SkeletalBodySetups.Num();
        Ar << ArraySize;
    
        if (Ar.IsLoading())
        {
            // 로드 시 배열 크기 설정
            SkeletalBodySetups.SetNum(ArraySize);
        }
        
        for (int i = 0; i < SkeletalBodySetups.Num(); ++i)
        {
    
            if (Ar.IsSaving())
            {
                FName Name = SkeletalBodySetups[i] ? SkeletalBodySetups[i]->GetFName() : NAME_None;
                Ar << Name;
    
                if (Name == NAME_None)
                {
                    assert(false);
                }
            }
            else
            {
                FName Name;
                Ar << Name;
                SkeletalBodySetups[i] = FObjectFactory::ConstructObject<USkeletalBodySetup>(this, Name);
            }
            
            SkeletalBodySetups[i]->Serialize(Ar);
        }
    }
    
    {
        // 배열 크기 직렬화
        int ArraySize = ConstraintTemplates.Num();
        Ar << ArraySize;
    
        if (Ar.IsLoading())
        {
            // 로드 시 배열 크기 설정
            ConstraintTemplates.SetNum(ArraySize);
        }
        
        for (int i = 0; i < ConstraintTemplates.Num(); ++i)
        {
    
            if (Ar.IsSaving())
            {
                FName ConstraintName = ConstraintTemplates[i] ? ConstraintTemplates[i]->GetFName() : NAME_None;
                Ar << ConstraintName;
    
                if (ConstraintName == NAME_None)
                {
                    assert(false);
                }
            }
            else
            {
                FName ConstraintName;
                Ar << ConstraintName;
                ConstraintTemplates[i] = FObjectFactory::ConstructObject<UPhysicsConstraintTemplate>(this, ConstraintName);
            }
            
            ConstraintTemplates[i]->Serialize(Ar);
        }
    }
}

void UPhysicsAsset::CreatePhysicsInstance(UWorld* World, USkeletalMeshComponent* SkeletalMeshComponent, TArray<FBodyInstance*>& OutBodies, TArray<FConstraintInstance*>& OutConstraints)
{
    if (!World || !SkeletalMeshComponent)
    {
        return;
    }

    UpdateBodySetupIndexMap();

    const int32 NumBodies = SkeletalBodySetups.Num();
    OutBodies.SetNum(NumBodies);

    // 컴포넌트 끼리의 충돌을 방지하기 위해 UUID를 사용.
    const uint32 ComponentId = SkeletalMeshComponent->GetUUID();

    const FReferenceSkeleton& RefSkeleton = SkeletalMeshComponent->GetSkeletalMeshAsset()->GetSkeleton()->GetReferenceSkeleton();
    TArray<FMatrix> GlobalBoneMatrices;
    SkeletalMeshComponent->GetCurrentGlobalBoneMatrices(GlobalBoneMatrices);
    FMatrix ComponentWolrdMatrix =  SkeletalMeshComponent->GetWorldMatrix();
    
    for (int i = 0; i < NumBodies; ++i)
    {
        USkeletalBodySetup* BodySetup = SkeletalBodySetups[i];
        if (!BodySetup)
        {
            continue;
        }

        const FName BoneName = BodySetup->BoneName;
        const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
        if (BoneIndex == INDEX_NONE)
        {
            continue; // 본이 유효하지 않으면 건너뜀
        }



        //FTransform GlobalBoneTransform = FTransform(GlobalBoneMatrices[BoneIndex]);

        const FMatrix LocalBoneTM = FMatrix(GlobalBoneMatrices[BoneIndex]);
        const FMatrix WorldBoneTM = LocalBoneTM * ComponentWolrdMatrix;
        
        FBodyInstance* NewBody = new FBodyInstance();
        NewBody->BodySetup = BodySetup;
        NewBody->InstanceBodyIndex = i;
        NewBody->ComponentId = ComponentId; // 컴포넌트 ID 설정
        NewBody->InitBody(World->GetPhysicsScene(), FTransform(WorldBoneTM), SkeletalMeshComponent);
        OutBodies[i] = NewBody;
    }

    for (UPhysicsConstraintTemplate* Template : ConstraintTemplates)
    {
        if (!Template)
        {
            continue;
        }

        const FName& ChildBone = Template->DefaultInstance.ConstraintBone1;
        const FName& ParentBone = Template->DefaultInstance.ConstraintBone2;

        const int32 ChildIndex = BodySetupIndexMap.FindRef(ChildBone);
        const int32 ParentIndex = BodySetupIndexMap.FindRef(ParentBone);

        if (!OutBodies.IsValidIndex(ChildIndex) || !OutBodies.IsValidIndex(ParentIndex))
        {
            continue; // Child body setup이 유효하지 않으면 건너뜀
        }

        FBodyInstance* Body1 = OutBodies[ChildIndex];
        FBodyInstance* Body2 = OutBodies[ParentIndex];
        if (!Body1 || !Body2)
        {
            continue; // Body가 유효하지 않으면 건너뜀
        }

        FConstraintInstance* NewConstraint = new FConstraintInstance();
        *NewConstraint = Template->DefaultInstance;
        NewConstraint->InitConstraint(Body1, Body2);
        OutConstraints.Add(NewConstraint);
    }
}
//
//bool UPhysicsAsset::AddConstrainBetweenBones(FName FirstBoneName, FName SecondBoneName, USkeletalMesh* SkeletalMesh)
//{
//    if (!SkeletalMesh || !SkeletalMesh->GetSkeleton())
//    {
//        UE_LOG(ELogLevel::Error, TEXT("Invalid SkeletalMesh or Skeleton."));
//        return false;
//    }
//
//    const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetSkeleton()->GetReferenceSkeleton();
//    const int32 FirstBoneIndex = RefSkeleton.FindBoneIndex(FirstBoneName);
//    const int32 SecondBoneIndex = RefSkeleton.FindBoneIndex(SecondBoneName);
//
//    if (FirstBoneIndex == INDEX_NONE || SecondBoneIndex == INDEX_NONE)
//    {
//        UE_LOG(ELogLevel::Error, TEXT("Invalid bone names: %s or %s."), *FirstBoneName.ToString(), *SecondBoneName.ToString());
//        return false;
//    }
//
//    // 중복 체크.
//    for (const UPhysicsConstraintTemplate* Template : ConstraintTemplates)
//    {
//        if (Template &&
//            Template->DefaultInstance.ConstraintBone1 == FirstBoneName &&
//            Template->DefaultInstance.ConstraintBone2 == SecondBoneName)
//        {
//            return false;
//        }
//    }
//
//    UPhysicsConstraintTemplate* NewConstraint = FObjectFactory::ConstructObject<UPhysicsConstraintTemplate>(this);
//    if (!NewConstraint)
//    {
//        return false;
//    }
//
//    NewConstraint->DefaultInstance.ConstraintBone1 = FirstBoneName;
//    NewConstraint->DefaultInstance.ConstraintBone2 = SecondBoneName;
//    NewConstraint->DefaultInstance.JointName = FName(*FString::Printf(TEXT("[%s -> %s] Constraint"), *FirstBoneName.ToString(), *SecondBoneName.ToString()));
//
//    const FTransform FirstBoneTM = RefSkeleton.GetRawRefBonePose()[FirstBoneIndex];
//    const FTransform SecondBoneTM = RefSkeleton.GetRawRefBonePose()[SecondBoneIndex];
//    const FTransform RelativeTM = FirstBoneTM.GetRelativeTransform(SecondBoneTM);
//
//    NewConstraint->DefaultInstance.Frame1.Position = FVector::ZeroVector; // Frame1은 항상 child 기준
//    NewConstraint->DefaultInstance.Frame1.PriAxis = FVector::ForwardVector; // X축 (Twist)
//    NewConstraint->DefaultInstance.Frame1.SecAxis = FVector::RightVector; // Y축 (Swing1)
//
//    NewConstraint->DefaultInstance.Frame2.Position = RelativeTM.GetTranslation();
//    const FQuat RelativeRot = RelativeTM.GetRotation();
//    NewConstraint->DefaultInstance.Frame2.PriAxis = 
//
//    ConstraintTemplates.Add(NewConstraint);
//
//    return true;
//}
