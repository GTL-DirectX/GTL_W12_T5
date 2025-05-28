#include "SkeletalMeshActor.h"

#include "Engine/AssetManager.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/Asset/SkeletalMeshAsset.h"

ASkeletalMeshActor::ASkeletalMeshActor()
{
    
}

void ASkeletalMeshActor::PostSpawnInitialize()
{
    AActor::PostSpawnInitialize();

    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>(TEXT("DefaultSkMeshComp"));
    
    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh(TEXT("Contents/Human/Human")));

}

UObject* ASkeletalMeshActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}
