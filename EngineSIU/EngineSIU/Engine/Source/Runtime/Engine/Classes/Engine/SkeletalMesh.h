#pragma once
#include "SkinnedAsset.h"
#include "Asset/SkeletalMeshAsset.h" 
class USkeleton;
//struct FSkeletalMeshRenderData;

class USkeletalMesh : public USkinnedAsset
{
    DECLARE_CLASS(USkeletalMesh, USkinnedAsset)

public:
    USkeletalMesh() = default;
    virtual ~USkeletalMesh() override = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    void SetRenderData(std::unique_ptr<FSkeletalMeshRenderData> InRenderData);

    const FSkeletalMeshRenderData* GetRenderData() const;

    USkeleton* GetSkeleton() const { return Skeleton; }

    void SetSkeleton(USkeleton* InSkeleton) { Skeleton = InSkeleton; }

    virtual void SerializeAsset(FArchive& Ar) override;

    void CreateOrBindPhysicsAsset();

    class UPhysicsAsset* PhysicsAsset = nullptr;
  
protected:
    std::unique_ptr<FSkeletalMeshRenderData> RenderData;

    USkeleton* Skeleton;
};
