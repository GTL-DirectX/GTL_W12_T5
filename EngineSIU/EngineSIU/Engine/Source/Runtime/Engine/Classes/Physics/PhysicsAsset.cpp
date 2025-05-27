#include "PhysicsAsset.h"

#include "PhysicsConstraintTemplate.h"
#include "UObject/ObjectFactory.h"

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
    
                if (ConstraintName != NAME_None)
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
