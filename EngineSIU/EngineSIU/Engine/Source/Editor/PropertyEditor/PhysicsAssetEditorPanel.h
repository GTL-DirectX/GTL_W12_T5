#pragma once

#include "UnrealEd/EditorPanel.h" // 기존 EditorPanel 상속
#include "Math/Transform.h"      // FTransform 등 사용
#include "UObject/NameTypes.h"   // FName 사용

// PhysX 관련 헤더 (필요에 따라 추가)
// #include <PxPhysicsAPI.h>

// 전방 선언
class USkeletalMesh;
class UPhysicsAsset; // 엔진의 피직스 에셋 데이터 구조체 (가칭)
struct FPhysicsBodyData; // 피직스 바디 데이터 구조체 (가칭)
struct FConstraintData;  // 컨스트레인트 데이터 구조체 (가칭)
struct FReferenceSkeleton; // 스켈레톤 참조 구조체

// 피직스 바디 타입 (예시)
enum class EPhysicsBodyType : uint8
{
    Capsule,
    Box,
    Sphere
};

class PhysicsAssetEditorPanel : public UEditorPanel
{
public:
    PhysicsAssetEditorPanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);
    void SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset); // 작업할 피직스 에셋 설정

private:
    float Width = 0, Height = 0;

    USkeletalMesh* CurrentSkeletalMesh = nullptr;
    UPhysicsAsset* CurrentPhysicsAsset = nullptr; // 현재 편집 중인 피직스 에셋

    // 선택된 항목 정보
    int32 SelectedBoneIndex = INDEX_NONE;
    FPhysicsBodyData* SelectedBodyData = nullptr;   // 선택된 바디의 데이터 포인터
    FConstraintData* SelectedConstraintData = nullptr; // 선택된 컨스트레인트의 데이터 포인터
    FName SelectedItemName; // 선택된 항목의 이름 (본, 바디, 컨스트레인트)

    // UI 레이아웃 크기 비율
    float ToolbarHeightRatio = 0.05f;
    float LeftPanelWidthRatio = 0.25f; // 스켈레톤 트리 및 툴 영역
    float DetailPanelWidthRatio = 0.25f; // 속성 편집 영역 (우측)
    // 중앙은 3D 뷰포트 영역으로 가정

    // 임시 데이터 (새 바디/컨스트레인트 생성 시)
    EPhysicsBodyType NewBodyType = EPhysicsBodyType::Capsule;
    int32 ConstraintParentBodyIndex = INDEX_NONE; // 컨스트레인트 생성 시 첫 번째 선택 바디
    int32 ConstraintChildBodyIndex = INDEX_NONE;  // 컨스트레인트 생성 시 두 번째 선택 바디

    // --- 렌더링 함수 ---
    void RenderToolbar();
    void RenderSkeletonTreePanel(); // 스켈레톤, 바디, 컨스트레인트 트리
    void RenderDetailPanel();       // 선택된 항목의 속성 편집

    // --- 스켈레톤 트리 헬퍼 ---
    void RenderBoneNode(const FReferenceSkeleton& RefSkeleton, int32 BoneIndex);
    void RenderBodyNode(FPhysicsBodyData* BodyData, int32 BodyIndex); // 바디 노드 렌더링
    void RenderConstraintNode(FConstraintData* ConstraintData, int32 ConstraintIndex); // 컨스트레인트 노드 렌더링

    // --- 디테일 패널 헬퍼 ---
    void RenderBoneDetails();
    void RenderBodyDetails();
    void RenderConstraintDetails();

    // --- 액션 함수 ---
    void AddNewBodyToSelectedBone();
    void RemoveSelectedBody();
    void AddNewConstraintBetweenSelectedBodies(); // (선택된 두 바디를 기반으로)
    void RemoveSelectedConstraint();
    void ClearSelection(bool bClearBone = true);

    // --- 유틸리티 ---
    FString GetCleanBoneName(const FString& InFullName); // SkeletalMeshViewerPanel에서 가져옴
    // 필요한 경우 PhysX 관련 데이터 접근 함수
};
