#include "PhysicsAsset.h"

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
