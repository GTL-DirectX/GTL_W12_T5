#pragma once

#include "CoreMiscDefines.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UnrealEd/EditorPanel.h" // UEditorPanel 상속
#include "UObject/NameTypes.h"

class USkeletalMesh;
struct FReferenceSkeleton;
// Forward declarations
class UPhysicsAsset;
class USkeletalBodySetup;
class UPhysicsConstraintTemplate;
class UEditorEngine;

class PhysicsAssetEditorPanel : public UEditorPanel
{
public:
    PhysicsAssetEditorPanel();
    virtual ~PhysicsAssetEditorPanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    // 현재 편집 중인 피직스 에셋을 설정하는 함수
    void SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset);
    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh); // 스켈레탈 메시 설정 함수 추가


private:
    // 창 크기 관련 변수
    float Width = 0, Height = 0;

    // 현재 편집 중인 피직스 에셋
    UPhysicsAsset* CurrentPhysicsAsset = nullptr;
    USkeletalMesh* CurrentSkeletalMesh = nullptr; // 현재 참조하는 스켈레탈 메시

    // 선택된 항목 정보 (단순화를 위해 인덱스나 이름으로 관리)
    FName SelectedBoneName = NAME_None; // 본 자체를 선택했을 때
    USkeletalBodySetup* SelectedBodySetup = nullptr; // 선택된 BodySetup 객체
    UPhysicsConstraintTemplate* SelectedConstraintTemplate = nullptr; // 선택된 ConstraintTemplate 객체

    void RenderDetailsPanel();
    void RenderObjectDetails(UObject* SelectedObject); // UObject의 프로퍼티를 표시하는 헬퍼 함수
    void RenderConstraintCreationUI(FName FirstBoneName, const FString& UiContextId);

    // 스켈레톤 트리 렌더링 함수
    void RenderSkeletonTree();
    void RenderBoneNodeRecursive(const FReferenceSkeleton& RefSkeleton, int32 BoneIndex);

    // 아이콘 로드 (필요하다면)
    ID3D11ShaderResourceView* BoneIconSRV = nullptr;
    ID3D11ShaderResourceView* BodyIconSRV = nullptr;
    ID3D11ShaderResourceView* ConstraintIconSRV = nullptr;
    void LoadIcons();

    // ImGui ID 관리를 위한 헬퍼
    int32 ImGuiNodeIdCounter = 0;
    int32 GetUniqueNodeID() { return ImGuiNodeIdCounter++; }
};



