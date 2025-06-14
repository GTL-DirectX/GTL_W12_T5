#pragma once
#include "StaticMesh.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UPhysicsAsset;
class UParticleSystem;
class UAnimationAsset;
class USkeleton;
class USkeletalMesh;

enum class EAssetType : uint8
{
    StaticMesh,
    SkeletalMesh,
    Skeleton,
    Animation,
    Texture2D,
    Material,
    ParticleSystem,
    PhysicsAsset,
};

struct FAssetInfo
{
    FName AssetName;        // Asset의 이름
    FName PackagePath;      // Asset의 패키지 경로
    FString SourceFilePath; // 원본 파일 경로
    EAssetType AssetType;   // Asset의 타입
    uint32 Size;            // Asset의 크기 (바이트 단위)

    [[nodiscard]] FString GetFullPath() const { return PackagePath.ToString() / AssetName.ToString(); }

    void Serialize(FArchive& Ar)
    {
        int8 Type = static_cast<int8>(AssetType);

        Ar << AssetName
           << PackagePath
           << SourceFilePath
           << Type
           << Size;

        AssetType = static_cast<EAssetType>(Type);
    }
};

struct FAssetRegistry
{
    TMap<FName, FAssetInfo> PathNameToAssetInfo;
};

struct FAssetLoadResult
{
    TArray<USkeleton*> Skeletons;
    TArray<USkeletalMesh*> SkeletalMeshes;
    TArray<UStaticMesh*> StaticMeshes;
    TArray<UMaterial*> Materials;
    TArray<UAnimationAsset*> Animations;
    TArray<UPhysicsAsset*> PhysicsAssets;
};

class UAssetManager : public UObject
{
    DECLARE_CLASS(UAssetManager, UObject)

private:
    std::unique_ptr<FAssetRegistry> AssetRegistry;

public:
    UAssetManager() = default;
    virtual ~UAssetManager() override = default;

    static bool IsInitialized();

    /** UAssetManager를 가져옵니다. */
    static UAssetManager& Get();

    /** UAssetManager가 존재하면 가져오고, 없으면 nullptr를 반환합니다. */
    static UAssetManager* GetIfInitialized();
    
    void InitAssetManager();

    const TMap<FName, FAssetInfo>& GetAssetRegistry();
    TMap<FName, FAssetInfo>& GetAssetRegistryRef();

    UObject* GetAsset(EAssetType AssetType, const FName& Name) const;
    USkeletalMesh* GetSkeletalMesh(const FName& Name) const;
    UStaticMesh* GetStaticMesh(const FName& Name) const;
    USkeleton* GetSkeleton(const FName& Name) const;
    UMaterial* GetMaterial(const FName& Name) const;
    UAnimationAsset* GetAnimation(const FName& Name) const;
    UParticleSystem* GetParticleSystem(const FName& Name) const;
    UPhysicsAsset* GetPhysicsAsset(const FName& Name) const;

    void GetMaterialKeys(TSet<FName>& OutKeys) const;
    void GetMaterialKeys(TArray<FName>& OutKeys) const;

    void AddAssetInfo(const FAssetInfo& Info);
    void AddSkeleton(const FName& Key, USkeleton* Skeleton);
    void AddSkeletalMesh(const FName& Key, USkeletalMesh* SkeletalMesh);
    void AddMaterial(const FName& Key, UMaterial* Material);
    void AddStaticMesh(const FName& Key, UStaticMesh* StaticMesh);
    void AddAnimation(const FName& Key, UAnimationAsset* Animation);
    void AddParticleSystem(const FName& Key, UParticleSystem* ParticleSystem);
    
    void SavePhysicsAssets(UPhysicsAsset* PhysicsAsset, const FString& InAssetPath);


private:
    inline static TMap<EAssetType, TMap<FName, UObject*>> AssetMap;
    
    double FbxLoadTime = 0.0;
    double BinaryLoadTime = 0.0;

    void LoadContentFiles();
    
    void HandleFBX(const FAssetInfo& AssetInfo);
    
    void LoadPhysicsAsset(const FAssetInfo& AssetInfo);

    void AddToAssetMap(const FAssetLoadResult& Result, const FString& BaseName, const FAssetInfo& BaseAssetInfo);

    bool LoadFbxBinary(const FString& FilePath, FAssetLoadResult& Result, const FString& BaseName, const FString& FolderPath);

    bool LoadPhysicsAssetBinary(const FString& FilePath, FAssetLoadResult& Result, const FString& BaseName, const FString& FolderPath);
    
    bool SaveFbxBinary(const FString& FilePath, FAssetLoadResult& Result, const FString& BaseName, const FString& FolderPath);

    bool SavePhysicsAssetBinary(const FString& FilePath, FAssetLoadResult& Result, const FString& BaseName, const FString& FolderPath);
    
    static constexpr uint32 Version = 1;

    bool SerializeVersion(FArchive& Ar);

    
    bool SerializeAssetLoadResult(FArchive& Ar, FAssetLoadResult& Result, const FString& BaseName, const FString& FolderPath);
};


