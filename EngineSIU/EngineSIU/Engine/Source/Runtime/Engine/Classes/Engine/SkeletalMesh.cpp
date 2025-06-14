
#include "Asset/SkeletalMeshAsset.h"
#include "SkeletalMesh.h"
#include "UObject/Casts.h"

#include "AssetManager.h"
#include "Physics/PhysicsAsset.h"
#include "UObject/ObjectFactory.h"

UObject* USkeletalMesh::Duplicate(UObject* InOuter)
{
    ThisClass* NewObject = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewObject->PhysicsAsset = PhysicsAsset;

    return NewObject;
}

void USkeletalMesh::SetRenderData(std::unique_ptr<FSkeletalMeshRenderData> InRenderData)
{
    RenderData = std::move(InRenderData);
}

const FSkeletalMeshRenderData* USkeletalMesh::GetRenderData() const
{
    return RenderData.get(); 
}

void USkeletalMesh::SerializeAsset(FArchive& Ar)
{
    if (Ar.IsLoading())
    {
        if (!RenderData)
        {
            RenderData = std::make_unique<FSkeletalMeshRenderData>();
        }
    }

    RenderData->Serialize(Ar);
}

void USkeletalMesh::CreateOrBindPhysicsAsset()
{
    if (!RenderData || RenderData->ObjectName.empty())
    {
        return;
    }
    PhysicsAsset = UAssetManager::Get().GetPhysicsAsset(RenderData->ObjectName + TEXT("_PhysicsAsset.physbin"));

    if (PhysicsAsset == nullptr)
    {
        PhysicsAsset = FObjectFactory::ConstructObject<UPhysicsAsset>(this, RenderData->ObjectName + TEXT("_PhysicsAsset"));
    }
}


