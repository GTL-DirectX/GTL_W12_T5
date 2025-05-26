#include "PhysicsAssetEditorPanel.h"
#include "Engine/EditorEngine.h" // GEngine 등 사용
#include "Engine/SkeletalMesh.h" // USkeletalMesh
#include "Animation/Skeleton.h"  // USkeleton, FReferenceSkeleton
// #include "PhysicsEngine/PhysicsAsset.h" // UPhysicsAsset (가칭)
// #include "PhysicsEngine/BodySetup.h"    // FPhysicsBodyData (가칭)
// #include "PhysicsEngine/ConstraintInstance.h" // FConstraintData (가칭)

// --- 임시 데이터 구조체 (실제 엔진의 구조체를 사용해야 함) ---
struct FPhysicsBodyData {
    FName AttachedBoneName;
    EPhysicsBodyType BodyType = EPhysicsBodyType::Capsule;
    FTransform LocalTransform; // 본으로부터의 상대적 트랜스폼
    // 캡슐
    float CapsuleRadius = 10.0f;
    float CapsuleHalfHeight = 20.0f;
    // 박스
    FVector BoxHalfExtents = FVector(10.0f, 10.0f, 10.0f);
    // 스피어
    float SphereRadius = 10.0f;
    float Mass = 1.0f;
    // ... 기타 물리 재질, 충돌 설정 등
    int32 UniqueID; // UI에서 선택 등을 위한 고유 ID
    // PxRigidActor* PhysXActor; // 실제 PhysX 액터 포인터 (엔진에서 관리)
    bool operator==(const FPhysicsBodyData& Other) const
    {
        // UniqueID가 같다면 두 객체는 같은 것으로 간주
        return UniqueID == Other.UniqueID;
    }
};

struct FConstraintData {
    FName ParentBodyName; // 또는 Body Index
    FName ChildBodyName;  // 또는 Body Index
    FTransform LocalFrameParent; // 부모 바디 기준 로컬 프레임
    FTransform LocalFrameChild;  // 자식 바디 기준 로컬 프레임
    // D6 조인트 제한 값들
    // PxD6Motion::Enum TwistMotion, Swing1Motion, Swing2Motion;
    // PxJointAngularLimitPair TwistLimit;
    // PxJointLimitCone SwingLimit;
    // ... 기타 선형 제한, 드라이브 등
    int32 UniqueID;
    // PxJoint* PhysXJoint; // 실제 PhysX 조인트 포인터 (엔진에서 관리)
    bool operator==(const FConstraintData& Other) const
    {
        // UniqueID가 같다면 두 객체는 같은 것으로 간주
        return UniqueID == Other.UniqueID;
    }
};

// UPhysicsAsset (가칭)
class UPhysicsAsset {
public:
    TArray<FPhysicsBodyData> Bodies;
    TArray<FConstraintData> Constraints;
    // ...
};
// --- 임시 데이터 구조체 끝 ---


PhysicsAssetEditorPanel::PhysicsAssetEditorPanel()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::PhysicsViewer| EWorldTypeBitFlag::SimulationViewer);
}

void PhysicsAssetEditorPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine || !CurrentSkeletalMesh || !CurrentPhysicsAsset)
    {
        // 필요한 에셋이 로드되지 않았으면 아무것도 렌더링하지 않거나 안내 메시지 표시
        ImGui::Text("Please load a Skeletal Mesh and create/load a Physics Asset.");
        return;
    }

    RenderToolbar();

    float CurrentY = Height * ToolbarHeightRatio;
    float RemainingHeight = Height * (1.0f - ToolbarHeightRatio);

    // 좌측 패널 (스켈레톤 트리)
    ImGui::SetNextWindowPos(ImVec2(0, CurrentY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(Width * LeftPanelWidthRatio, RemainingHeight), ImGuiCond_Always);
    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    RenderSkeletonTreePanel();
    ImGui::End();

    // 우측 패널 (디테일)
    ImGui::SetNextWindowPos(ImVec2(Width * (1.0f - DetailPanelWidthRatio), CurrentY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(Width * DetailPanelWidthRatio, RemainingHeight), ImGuiCond_Always);
    ImGui::Begin("Details", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    RenderDetailPanel();
    ImGui::End();

    // 중앙 3D 뷰포트 영역은 ImGui 외부에서 엔진이 직접 렌더링한다고 가정
    // ImGui::SetNextWindowPos(ImVec2(Width * LeftPanelWidthRatio, CurrentY), ImGuiCond_Always);
    // ImGui::SetNextWindowSize(ImVec2(Width * (1.0f - LeftPanelWidthRatio - DetailPanelWidthRatio), RemainingHeight), ImGuiCond_Always);
    // ImGui::Begin("Viewport");
    // // 여기에 3D 렌더 타겟 텍스처를 ImGui::Image()로 표시
    // ImGui::End();
}

void PhysicsAssetEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = static_cast<float>(ClientRect.right - ClientRect.left);
    Height = static_cast<float>(ClientRect.bottom - ClientRect.top);
}

void PhysicsAssetEditorPanel::SetSkeletalMesh(USkeletalMesh* InSkeletalMesh)
{
    CurrentSkeletalMesh = InSkeletalMesh;
    ClearSelection();
    // 새 스켈레탈 메시에 맞는 PhysicsAsset을 생성하거나 로드하는 로직 필요
    if (CurrentSkeletalMesh && !CurrentPhysicsAsset) {
        // CurrentPhysicsAsset = CreateNewPhysicsAssetForMesh(CurrentSkeletalMesh); // 예시
    }
}

void PhysicsAssetEditorPanel::SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset)
{
    CurrentPhysicsAsset = InPhysicsAsset;
    ClearSelection();
}

void PhysicsAssetEditorPanel::RenderToolbar()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(Width, Height * ToolbarHeightRatio), ImGuiCond_Always);
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button("Load Skeletal Mesh")) { /* 파일 다이얼로그 및 로드 로직 호출 */ }
    ImGui::SameLine();
    if (ImGui::Button("New Physics Asset")) { /* 새 피직스 에셋 생성 로직 */ }
    ImGui::SameLine();
    if (ImGui::Button("Load Physics Asset")) { /* 파일 다이얼로그 및 로드 로직 호출 */ }
    ImGui::SameLine();
    if (ImGui::Button("Save Physics Asset")) { /* 저장 로직 호출 */ }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    // 시뮬레이션 관련 버튼 (ParticleViewerPanel 참고)
    if (ImGui::Button("Simulate")) { /* 시뮬레이션 시작/정지 토글 */ }
    // 기타 툴 버튼들...

    ImGui::End();
}

void PhysicsAssetEditorPanel::RenderSkeletonTreePanel()
{
    if (!CurrentSkeletalMesh || !CurrentSkeletalMesh->GetSkeleton() || !CurrentPhysicsAsset) return;

    const FReferenceSkeleton& RefSkeleton = CurrentSkeletalMesh->GetSkeleton()->GetReferenceSkeleton();

    if (ImGui::CollapsingHeader("Skeleton Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (int32 i = 0; i < RefSkeleton.GetRawBoneNum(); ++i)
        {
            if (RefSkeleton.GetParentIndex(i) == INDEX_NONE) // 루트 본
            {
                RenderBoneNode(RefSkeleton, i);
            }
        }
    }

    if (ImGui::CollapsingHeader("Physics Bodies", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (int32 i = 0; i < CurrentPhysicsAsset->Bodies.Num(); ++i)
        {
            RenderBodyNode(&CurrentPhysicsAsset->Bodies[i], i);
        }
    }

    if (ImGui::CollapsingHeader("Constraints", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (int32 i = 0; i < CurrentPhysicsAsset->Constraints.Num(); ++i)
        {
            RenderConstraintNode(&CurrentPhysicsAsset->Constraints[i], i);
        }
    }
}

void PhysicsAssetEditorPanel::RenderBoneNode(const FReferenceSkeleton& RefSkeleton, int32 BoneIndex)
{
    const FMeshBoneInfo& BoneInfo = RefSkeleton.GetRawRefBoneInfo()[BoneIndex];
    FString BoneDisplayName = GetCleanBoneName(BoneInfo.Name.ToString());

    ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (SelectedBoneIndex == BoneIndex && !SelectedBodyData && !SelectedConstraintData) {
        NodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // 자식 찾기 (바디, 컨스트레인트 포함)
    bool bHasChildren = false;
    for (int32 i = 0; i < RefSkeleton.GetRawBoneNum(); ++i) {
        if (RefSkeleton.GetParentIndex(i) == BoneIndex) {
            bHasChildren = true;
            break;
        }
    }
    // 이 본에 연결된 바디가 있는지 확인 (단순화된 예시)
    for (const auto& Body : CurrentPhysicsAsset->Bodies) {
        if (Body.AttachedBoneName == BoneInfo.Name) {
            // bHasChildren = true; // 바디는 본의 자식으로 직접 표시하지 않고 별도 섹션에서 관리
            break;
        }
    }

    if (!bHasChildren) {
        NodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    ImGui::PushID(BoneIndex); // 고유 ID
    bool bNodeOpen = ImGui::TreeNodeEx(GetData(BoneDisplayName), NodeFlags);
    if (ImGui::IsItemClicked()) {
        ClearSelection();
        SelectedBoneIndex = BoneIndex;
        SelectedItemName = BoneInfo.Name;
    }

    // 컨텍스트 메뉴 (우클릭)
    if (ImGui::BeginPopupContextItem("BoneContextMenu")) {
        if (ImGui::MenuItem("Add New Body (Capsule)")) {
            ClearSelection(); SelectedBoneIndex = BoneIndex; SelectedItemName = BoneInfo.Name;
            NewBodyType = EPhysicsBodyType::Capsule; AddNewBodyToSelectedBone();
        }
        if (ImGui::MenuItem("Add New Body (Box)")) {
            ClearSelection(); SelectedBoneIndex = BoneIndex; SelectedItemName = BoneInfo.Name;
            NewBodyType = EPhysicsBodyType::Box; AddNewBodyToSelectedBone();
        }
        if (ImGui::MenuItem("Add New Body (Sphere)")) {
            ClearSelection(); SelectedBoneIndex = BoneIndex; SelectedItemName = BoneInfo.Name;
            NewBodyType = EPhysicsBodyType::Sphere; AddNewBodyToSelectedBone();
        }
        ImGui::EndPopup();
    }


    if (bNodeOpen) {
        // 자식 본 재귀 호출
        for (int32 i = 0; i < RefSkeleton.GetRawBoneNum(); ++i) {
            if (RefSkeleton.GetParentIndex(i) == BoneIndex) {
                RenderBoneNode(RefSkeleton, i);
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void PhysicsAssetEditorPanel::RenderBodyNode(FPhysicsBodyData* BodyData, int32 BodyIndex)
{
    if (!BodyData) return;
    // 바디 이름은 "Body_BoneName_Type" 등으로 생성하거나 고유 ID 사용
    FString BodyDisplayName = FString::Printf(TEXT("Body_%s_%d"), *BodyData->AttachedBoneName.ToString(), BodyData->UniqueID);

    ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_Leaf; // 바디는 보통 리프
    if (SelectedBodyData == BodyData) {
        NodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::PushID(BodyData->UniqueID + 10000); // 본 인덱스와 겹치지 않도록 ID 오프셋
    bool bNodeOpen = ImGui::TreeNodeEx(GetData(BodyDisplayName), NodeFlags); // 바디는 항상 펼쳐진 상태로
    if (ImGui::IsItemClicked()) {
        ClearSelection();
        SelectedBodyData = BodyData;
        SelectedItemName = FName(BodyDisplayName);
    }

    // 컨텍스트 메뉴
    if (ImGui::BeginPopupContextItem("BodyContextMenu")) {
        if (ImGui::MenuItem("Remove Body")) {
            ClearSelection(); SelectedBodyData = BodyData;
            RemoveSelectedBody();
            ImGui::CloseCurrentPopup(); // 메뉴 닫기 중요
            ImGui::PopID(); // PopID 먼저
            ImGui::TreePop(); // TreePop도
            return; // 아이템이 삭제되었으므로 더 이상 진행하지 않음
        }
        if (ImGui::MenuItem("Select for Constraint (Parent)")) {
            ConstraintParentBodyIndex = BodyIndex; // 실제로는 BodyData의 고유 ID나 포인터를 저장
        }
        if (ImGui::MenuItem("Select for Constraint (Child)")) {
            ConstraintChildBodyIndex = BodyIndex;
        }
        if (ConstraintParentBodyIndex != INDEX_NONE && ConstraintChildBodyIndex != INDEX_NONE && ConstraintParentBodyIndex != ConstraintChildBodyIndex) {
            if (ImGui::MenuItem("Create Constraint Between Selected")) {
                AddNewConstraintBetweenSelectedBodies();
                ConstraintParentBodyIndex = INDEX_NONE; // 선택 해제
                ConstraintChildBodyIndex = INDEX_NONE;
            }
        }
        ImGui::EndPopup();
    }

    if (bNodeOpen) { // 실제로는 항상 true (Leaf이므로)
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void PhysicsAssetEditorPanel::RenderConstraintNode(FConstraintData* ConstraintData, int32 ConstraintIndex)
{
    if (!ConstraintData) return;
    FString ConstraintDisplayName = FString::Printf(TEXT("Constraint_%s_to_%s_%d"), *ConstraintData->ParentBodyName.ToString(), *ConstraintData->ChildBodyName.ToString(), ConstraintData->UniqueID);

    ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_Leaf;
    if (SelectedConstraintData == ConstraintData) {
        NodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::PushID(ConstraintData->UniqueID + 20000);
    bool bNodeOpen = ImGui::TreeNodeEx(GetData(ConstraintDisplayName), NodeFlags);
    if (ImGui::IsItemClicked()) {
        ClearSelection();
        SelectedConstraintData = ConstraintData;
        SelectedItemName = FName(ConstraintDisplayName);
    }

    if (ImGui::BeginPopupContextItem("ConstraintContextMenu")) {
        if (ImGui::MenuItem("Remove Constraint")) {
            ClearSelection(); SelectedConstraintData = ConstraintData;
            RemoveSelectedConstraint();
            ImGui::CloseCurrentPopup();
            ImGui::PopID();
            ImGui::TreePop();
            return;
        }
        ImGui::EndPopup();
    }

    if (bNodeOpen) {
        ImGui::TreePop();
    }
    ImGui::PopID();
}


void PhysicsAssetEditorPanel::RenderDetailPanel()
{
    if (SelectedBodyData) {
        RenderBodyDetails();
    } else if (SelectedConstraintData) {
        RenderConstraintDetails();
    } else if (SelectedBoneIndex != INDEX_NONE) {
        RenderBoneDetails();
    } else {
        ImGui::Text("Select an item from the hierarchy to see details.");
    }
}

void PhysicsAssetEditorPanel::RenderBoneDetails()
{
    if (SelectedBoneIndex == INDEX_NONE || !CurrentSkeletalMesh || !CurrentSkeletalMesh->GetSkeleton()) return;
    const FReferenceSkeleton& RefSkeleton = CurrentSkeletalMesh->GetSkeleton()->GetReferenceSkeleton();
    const FMeshBoneInfo& BoneInfo = RefSkeleton.GetRawRefBoneInfo()[SelectedBoneIndex];

    ImGui::Text("Selected Bone: %s", GetData(GetCleanBoneName(BoneInfo.Name.ToString())));
    ImGui::Separator();
    // 본 관련 정보 표시 (예: 부모 본 이름, 로컬 트랜스폼 등 - 읽기 전용)
    ImGui::Text("Parent: %s", (BoneInfo.ParentIndex != INDEX_NONE) ? GetData(GetCleanBoneName(RefSkeleton.GetRawRefBoneInfo()[BoneInfo.ParentIndex].Name.ToString())) : "None");

    // 바디 추가 버튼들
    if (ImGui::Button("Add Capsule Body")) { NewBodyType = EPhysicsBodyType::Capsule; AddNewBodyToSelectedBone(); }
    if (ImGui::Button("Add Box Body")) { NewBodyType = EPhysicsBodyType::Box; AddNewBodyToSelectedBone(); }
    if (ImGui::Button("Add Sphere Body")) { NewBodyType = EPhysicsBodyType::Sphere; AddNewBodyToSelectedBone(); }
}

void PhysicsAssetEditorPanel::RenderBodyDetails()
{
    if (!SelectedBodyData) return;

    ImGui::Text("Selected Body: Body_%s_%d", *SelectedBodyData->AttachedBoneName.ToString(), SelectedBodyData->UniqueID);
    ImGui::Separator();

    // 바디 타입 선택
    const char* bodyTypes[] = { "Capsule", "Box", "Sphere" };
    int currentType = static_cast<int>(SelectedBodyData->BodyType);
    if (ImGui::Combo("Body Type", &currentType, bodyTypes, IM_ARRAYSIZE(bodyTypes))) {
        SelectedBodyData->BodyType = static_cast<EPhysicsBodyType>(currentType);
        // 타입 변경 시 PhysX 액터 재생성 필요
    }

    // 로컬 트랜스폼 편집 (FImGuiWidget::DrawVec3Control 등 활용)
    FVector Location = SelectedBodyData->LocalTransform.GetTranslation();
    FRotator Rotation = SelectedBodyData->LocalTransform.GetRotation().Rotator();
    // FImGuiWidget::DrawVec3Control("Local Offset", Location, ...);
    // FImGuiWidget::DrawRot3Control("Local Rotation", Rotation, ...);
    // SelectedBodyData->LocalTransform.SetTranslation(Location);
    // SelectedBodyData->LocalTransform.SetRotation(Rotation.Quaternion());

    // 타입별 크기 편집
    if (SelectedBodyData->BodyType == EPhysicsBodyType::Capsule) {
        ImGui::DragFloat("Radius", &SelectedBodyData->CapsuleRadius, 0.1f, 0.1f, 1000.0f);
        ImGui::DragFloat("Half Height", &SelectedBodyData->CapsuleHalfHeight, 0.1f, 0.1f, 1000.0f);
    } else if (SelectedBodyData->BodyType == EPhysicsBodyType::Box) {
        // ImGui::DragFloat3("Half Extents", SelectedBodyData->BoxHalfExtents.X, ...);
    } else if (SelectedBodyData->BodyType == EPhysicsBodyType::Sphere) {
        ImGui::DragFloat("Radius", &SelectedBodyData->SphereRadius, 0.1f, 0.1f, 1000.0f);
    }

    ImGui::DragFloat("Mass", &SelectedBodyData->Mass, 0.1f, 0.01f, 1000.0f);
    // ... 기타 물리 재질, 충돌 그룹 등 편집 UI

    if (ImGui::Button("Remove This Body")) {
        RemoveSelectedBody();
    }
}

void PhysicsAssetEditorPanel::RenderConstraintDetails()
{
    if (!SelectedConstraintData) return;

    ImGui::Text("Selected Constraint: Constraint_%s_to_%s_%d", *SelectedConstraintData->ParentBodyName.ToString(), *SelectedConstraintData->ChildBodyName.ToString(), SelectedConstraintData->UniqueID);
    ImGui::Separator();

    ImGui::Text("Parent Body: %s", *SelectedConstraintData->ParentBodyName.ToString());
    ImGui::Text("Child Body: %s", *SelectedConstraintData->ChildBodyName.ToString());

    // 로컬 프레임 편집 (부모, 자식)
    // FImGuiWidget::DrawTransformControl("Parent Local Frame", SelectedConstraintData->LocalFrameParent, ...);
    // FImGuiWidget::DrawTransformControl("Child Local Frame", SelectedConstraintData->LocalFrameChild, ...);

    ImGui::SeparatorText("D6 Joint Limits");
    // 각도 제한 (Twist, Swing1, Swing2)
    // const char* motionTypes[] = { "Locked", "Limited", "Free" };
    // ImGui::Combo("Twist Motion", (int*)&SelectedConstraintData->TwistMotion, motionTypes, IM_ARRAYSIZE(motionTypes));
    // if (SelectedConstraintData->TwistMotion == PxD6Motion::eLIMITED) {
    //    ImGui::DragFloatRange2("Twist Angle (Min/Max)", &SelectedConstraintData->TwistLimit.lower, &SelectedConstraintData->TwistLimit.upper, ...);
    // }
    // ... Swing1, Swing2 유사하게 구현

    // 선형 제한 (X, Y, Z) - 필요시 추가

    if (ImGui::Button("Remove This Constraint")) {
        RemoveSelectedConstraint();
    }
}

// --- 액션 함수 구현 (간단한 예시) ---
void PhysicsAssetEditorPanel::AddNewBodyToSelectedBone()
{
    if (SelectedBoneIndex == INDEX_NONE || !CurrentSkeletalMesh || !CurrentPhysicsAsset) return;
    const FReferenceSkeleton& RefSkeleton = CurrentSkeletalMesh->GetSkeleton()->GetReferenceSkeleton();
    const FMeshBoneInfo& BoneInfo = RefSkeleton.GetRawRefBoneInfo()[SelectedBoneIndex];

    FPhysicsBodyData NewBody;
    NewBody.AttachedBoneName = BoneInfo.Name;
    NewBody.BodyType = NewBodyType; // UI에서 설정된 타입 사용
    NewBody.LocalTransform = FTransform::Identity; // 기본값
    NewBody.UniqueID = 0; // 임시 고유 ID
    //NewBody.UniqueID = FMath::Rand(); // 임시 고유 ID
    // NewBody.PhysXActor = CreatePhysXActorForBody(NewBody); // PhysX 액터 생성

    CurrentPhysicsAsset->Bodies.Add(NewBody);
    SelectedBodyData = &CurrentPhysicsAsset->Bodies.Last(); // 새로 추가된 바디 선택
    SelectedBoneIndex = INDEX_NONE;
    SelectedConstraintData = nullptr;
    SelectedItemName = FName(FString::Printf(TEXT("Body_%s_%d"), *NewBody.AttachedBoneName.ToString(), NewBody.UniqueID));
}

void PhysicsAssetEditorPanel::RemoveSelectedBody()
{
    if (!SelectedBodyData || !CurrentPhysicsAsset) return;
    // 연결된 컨스트레인트도 함께 제거하는 로직 필요
    // DestroyPhysXActor(SelectedBodyData->PhysXActor);
    CurrentPhysicsAsset->Bodies.RemoveSingle(*SelectedBodyData); // 포인터 비교로 삭제 (주의: 실제로는 ID 기반으로 찾아야 안전)
    ClearSelection();
}

void PhysicsAssetEditorPanel::AddNewConstraintBetweenSelectedBodies()
{
    // ConstraintParentBodyIndex 와 ConstraintChildBodyIndex 를 사용하여
    // CurrentPhysicsAsset->Bodies 에서 해당 바디를 찾고 컨스트레인트 생성
    if (ConstraintParentBodyIndex == INDEX_NONE || ConstraintChildBodyIndex == INDEX_NONE ||
        !CurrentPhysicsAsset ||
        !CurrentPhysicsAsset->Bodies.IsValidIndex(ConstraintParentBodyIndex) ||
        !CurrentPhysicsAsset->Bodies.IsValidIndex(ConstraintChildBodyIndex))
    {
        return;
    }

    FPhysicsBodyData* ParentBody = &CurrentPhysicsAsset->Bodies[ConstraintParentBodyIndex];
    FPhysicsBodyData* ChildBody = &CurrentPhysicsAsset->Bodies[ConstraintChildBodyIndex];

    FConstraintData NewConstraint;
    NewConstraint.ParentBodyName = FName(FString::Printf(TEXT("Body_%s_%d"), *ParentBody->AttachedBoneName.ToString(), ParentBody->UniqueID));
    NewConstraint.ChildBodyName = FName(FString::Printf(TEXT("Body_%s_%d"), *ChildBody->AttachedBoneName.ToString(), ChildBody->UniqueID));
    NewConstraint.LocalFrameParent = FTransform::Identity; // 기본값 또는 계산된 값
    NewConstraint.LocalFrameChild = FTransform::Identity;
    NewConstraint.UniqueID = 0;
    //NewConstraint.UniqueID = FMath::Rand();
    // NewConstraint.PhysXJoint = CreatePhysXJoint(NewConstraint, ParentBody->PhysXActor, ChildBody->PhysXActor);

    CurrentPhysicsAsset->Constraints.Add(NewConstraint);
    SelectedConstraintData = &CurrentPhysicsAsset->Constraints.Last();
    ClearSelection(false); // 바디 선택은 유지하지 않음
    SelectedItemName = FName(FString::Printf(TEXT("Constraint_%s_to_%s_%d"), *NewConstraint.ParentBodyName.ToString(), *NewConstraint.ChildBodyName.ToString(), NewConstraint.UniqueID));

    ConstraintParentBodyIndex = INDEX_NONE;
    ConstraintChildBodyIndex = INDEX_NONE;
}


void PhysicsAssetEditorPanel::RemoveSelectedConstraint()
{
    if (!SelectedConstraintData || !CurrentPhysicsAsset) return;
    // DestroyPhysXJoint(SelectedConstraintData->PhysXJoint);
    CurrentPhysicsAsset->Constraints.RemoveSingle(*SelectedConstraintData);
    ClearSelection();
}

void PhysicsAssetEditorPanel::ClearSelection(bool bClearBone)
{
    if (bClearBone)
    {
        SelectedBoneIndex = INDEX_NONE;
    }
    SelectedBodyData = nullptr;
    SelectedConstraintData = nullptr;
    SelectedItemName = NAME_None;
}

FString PhysicsAssetEditorPanel::GetCleanBoneName(const FString& InFullName)
{
    // SkeletalMeshViewerPanel의 구현과 동일하게 사용
    int32 barIdx = InFullName.FindChar(TEXT('|'), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
    FString name = (barIdx != INDEX_NONE) ? InFullName.RightChop(barIdx + 1) : InFullName;
    int32 colonIdx = name.FindChar(TEXT(':'), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
    if (colonIdx != INDEX_NONE) {
        return name.RightChop(colonIdx + 1);
    }
    return name;
}
