#include "PhysicsAssetEditorPanel.h"
#include "Engine/EditorEngine.h" // UEditorEngine 사용
#include "ThirdParty/ImGui/include/ImGui/imgui.h" // ImGui 사용
// #include "Engine/Classes/PhysicsEngine/PhysicsAsset.h"
// #include "Engine/Classes/PhysicsEngine/BodySetup.h"
// #include "Engine/Classes/PhysicsEngine/PhysicsConstraintTemplate.h"
// #include "Engine/Classes/PhysicsEngine/ConstraintInstance.h" // FConstraintInstance 접근
#include "Engine/SkeletalMesh.h"
#include "Physics/ConstraintInstance.h"
#include "Physics/PhysicsAsset.h"
#include "Physics/PhysicsConstraintTemplate.h"

PhysicsAssetEditorPanel::PhysicsAssetEditorPanel()
{
    // 이 패널이 지원하는 월드 타입을 설정할 수 있습니다.
    SetSupportedWorldTypes(EWorldTypeBitFlag::PhysicsViewer | EWorldTypeBitFlag::SimulationViewer);
    LoadIcons();
}

PhysicsAssetEditorPanel::~PhysicsAssetEditorPanel()
{
    // 소멸자에서 리소스 정리 (예: 아이콘 SRV 해제)
}

void PhysicsAssetEditorPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Engine->GetSelectedComponent());

    if (SkeletalMeshComponent)
    {
        SetSkeletalMesh(SkeletalMeshComponent->GetSkeletalMeshAsset());
    }
    
    if (!Engine || !CurrentPhysicsAsset)
    {
        // 엔진이나 피직스 에셋이 없으면 아무것도 렌더링하지 않음
        // 또는 "피직스 에셋을 선택해주세요" 같은 메시지 표시 가능
        ImGui::Begin("Physics Asset Editor");
        ImGui::Text("No Physics Asset selected or loaded.");
        ImGui::End();
        return;
    }

    CurrentPhysicsAsset->UpdateBodySetupIndexMap();

    // --- 스켈레톤 트리 패널 렌더링 ---
    const float TreePanelWidth = Width * 0.25f;
    const float TreePanelHeight = Height; // 전체 높이 사용 또는 조절

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(TreePanelWidth, TreePanelHeight), ImGuiCond_Always);

    constexpr ImGuiWindowFlags TreePanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;

    ImGui::Begin("Physics Hierarchy", nullptr, TreePanelFlags);
    {
        ImGuiNodeIdCounter = 0;
        RenderSkeletonTree(); // 이 함수 내부에서 SelectedBodySetup, SelectedConstraintTemplate이 업데이트됨
    }
    ImGui::End();

    // --- 디테일 패널 렌더링 ---
    const float DetailsPanelPosX = TreePanelWidth; // 트리 패널 바로 오른쪽에서 시작
    const float DetailsPanelWidth = Width - TreePanelWidth; // 전체 너비에서 트리 패널 너비를 뺀 나머지
    const float DetailsPanelHeight = Height; // 전체 창 높이 사용

    ImGui::SetNextWindowPos(ImVec2(DetailsPanelPosX, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(DetailsPanelWidth, DetailsPanelHeight), ImGuiCond_Always);

    constexpr ImGuiWindowFlags DetailsPanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::Begin("Details", nullptr, DetailsPanelFlags);
    {
        RenderDetailsPanel();
    }
    ImGui::End();

    // 여기에 다른 패널들 (예: 디테일 패널, 뷰포트 등) 렌더링 코드 추가
    // ...
}

void PhysicsAssetEditorPanel::SetSkeletalMesh(USkeletalMesh* InSkeletalMesh)
{
    CurrentSkeletalMesh = InSkeletalMesh;
    SetPhysicsAsset(CurrentSkeletalMesh ? CurrentSkeletalMesh->PhysicsAsset : nullptr);
    
    SelectedBoneName = NAME_None;
    USkeletalBodySetup* SelectedBodySetup = nullptr; // 선택된 BodySetup 객체
    UPhysicsConstraintTemplate* SelectedConstraintTemplate = nullptr; // 선택된 ConstraintTemplate 객체
}

void PhysicsAssetEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = static_cast<float>(ClientRect.right - ClientRect.left);
    Height = static_cast<float>(ClientRect.bottom - ClientRect.top);
}

void PhysicsAssetEditorPanel::SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset)
{
    CurrentPhysicsAsset = InPhysicsAsset;
    // 피직스 에셋이 변경되면 선택 정보 초기화
    SelectedBoneName = NAME_None;
    USkeletalBodySetup* SelectedBodySetup = nullptr; // 선택된 BodySetup 객체
    UPhysicsConstraintTemplate* SelectedConstraintTemplate = nullptr; // 선택된 ConstraintTemplate 객체

    if (CurrentPhysicsAsset)
    {
        // BodySetupIndexMap이 최신 상태인지 확인하거나 업데이트
        CurrentPhysicsAsset->UpdateBodySetupIndexMap();
    }
}

void PhysicsAssetEditorPanel::RenderDetailsPanel()
{
    if (SelectedBodySetup)
    {
        ImGui::SeparatorText(*FString::Printf(TEXT("Body Setup: %s"), *SelectedBodySetup->BoneName.ToString()));
        RenderObjectDetails(SelectedBodySetup);

        // USkeletalBodySetup의 멤버인 UBodySetup의 프로퍼티도 표시하고 싶다면:
        // UBodySetup* BaseBodySetup = Cast<UBodySetup>(SelectedBodySetup); // 이미 USkeletalBodySetup이 UBodySetup을 상속
        // if (BaseBodySetup) {
        //     ImGui::SeparatorText(*FString::Printf(TEXT("Base BodySetup Properties (%s)"), *BaseBodySetup->GetClass()->GetName()));
        //     RenderObjectDetails(BaseBodySetup); // 이렇게 하면 중복될 수 있으니, RenderObjectDetails가 부모 클래스까지 처리하도록 하거나,
        // 명시적으로 필요한 프로퍼티만 표시
        // }

        // FKAggregateGeom 같은 커스텀 구조체는 별도의 UI 렌더링 함수 필요
        // 예: RenderAggregateGeomDetails(SelectedBodySetup->GetAggGeom());
    }
    else if (SelectedConstraintTemplate)
    {
        const FConstraintInstance& CI = SelectedConstraintTemplate->DefaultInstance;
        FString ConstraintName = CI.JointName != NAME_None ? CI.JointName.ToString() : FString::Printf(TEXT("%s - %s"), *CI.ConstraintBone1.ToString(), *CI.ConstraintBone2.ToString());

        ImGui::SeparatorText(*FString::Printf(TEXT("Constraint: %s"), *ConstraintName));
        RenderObjectDetails(SelectedConstraintTemplate); // UPhysicsConstraintTemplate의 프로퍼티

        // // FConstraintInstance는 UObject가 아니므로 직접 RenderObjectDetails를 사용할 수 없음.
        // // FConstraintInstance의 멤버들을 직접 ImGui 위젯으로 표시해야 함.
        // ImGui::SeparatorText("Constraint Instance Details");
        // // 예시:
        // ImGui::Text("Constraint Bone 1: %s", *CI.ConstraintBone1.ToString());
        // ImGui::Text("Constraint Bone 2: %s", *CI.ConstraintBone2.ToString());
        // // ... FConstraintFrame, ELinearConstraintMotion 등을 위한 커스텀 UI 로직 ...
        // // FImGuiWidget::DrawEnumCombo("X Motion", CI.XMotion); // 이런 헬퍼 함수가 있다면 사용
        // // FImGuiWidget::DrawFloat("Linear Limit", CI.Limits.Linear);
    }
    else if (SelectedBoneName != NAME_None)
    {
        ImGui::SeparatorText(*FString::Printf(TEXT("Bone: %s"), *SelectedBoneName.ToString()));
        // 본 자체에 대한 정보 표시 (예: 트랜스폼, 스켈레탈 메시에서의 정보 등)
        // 이 정보는 UObject가 아닐 수 있으므로 직접 ImGui 위젯 사용
        ImGui::Text("Selected bone does not have direct editable properties here.");
        ImGui::Text("Select an associated Body or Constraint to see details.");
    }
    else if (CurrentPhysicsAsset && !SelectedBodySetup && !SelectedConstraintTemplate && SelectedBoneName == NAME_None)
    {
        // 아무것도 선택되지 않았지만 피직스 에셋 자체는 로드된 경우
        ImGui::SeparatorText(*FString::Printf(TEXT("Physics Asset: %s"), *CurrentPhysicsAsset->GetName()));
        RenderObjectDetails(CurrentPhysicsAsset);
    }
    else
    {
        ImGui::Text("Select an item from the hierarchy to see details.");
    }

}

void PhysicsAssetEditorPanel::RenderObjectDetails(UObject* SelectedObject)
{
    if (!SelectedObject)
    {
        return;
    }

    const UClass* Class = SelectedObject->GetClass();
    for (; Class; Class = Class->GetSuperClass()) // 부모 클래스의 프로퍼티까지 순회
    {
        const TArray<FProperty*>& Properties = Class->GetProperties();
        if (!Properties.IsEmpty())
        {
            // 현재 클래스 이름으로 섹션 구분 (선택 사항)
            // if (Class == SelectedObject->GetClass() || Properties.ContainsByPredicate([](const FProperty* P){ return P->HasAnyPropertyFlags(CPF_Edit); }))
            // 위 조건은 Edit 플래그가 있는 프로퍼티가 있을 때만 클래스 이름을 표시하려는 의도일 수 있습니다.
            ImGui::SeparatorText(*FString::Printf(TEXT("%s Properties"), *Class->GetName()));
        }

        for (const FProperty* Prop : Properties)
        {
            Prop->DisplayInImGui(SelectedObject);
        }
    }
}

void PhysicsAssetEditorPanel::RenderSkeletonTree()
{
    if (!CurrentPhysicsAsset)
    {
        ImGui::Text("No Physics Asset loaded.");
        return;
    }

    const FReferenceSkeleton& RefSkeleton = CurrentSkeletalMesh->GetSkeleton()->GetReferenceSkeleton();

    if (RefSkeleton.GetRawBoneNum() == 0)
    {
        ImGui::Text("Skeleton has no bones.");
        return;
    }

    // 루트 본부터 시작하여 트리 렌더링
    for (int32 i = 0; i < RefSkeleton.GetRawBoneNum(); ++i)
    {
        if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == INDEX_NONE) // 루트 본인 경우
        {
            RenderBoneNodeRecursive(RefSkeleton, i);
        }
    }
}
void PhysicsAssetEditorPanel::RenderBoneNodeRecursive(const FReferenceSkeleton& RefSkeleton, int32 BoneIndex)
{
    ImGui::PushID(GetUniqueNodeID()); // 각 노드에 고유 ID 부여

    const FName CurrentBoneName = RefSkeleton.GetBoneName(BoneIndex);
    FString BoneLabel = FString::Printf(TEXT("%s"), *CurrentBoneName.ToString());

    ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

    // 현재 본에 연결된 BodySetup 찾기
    USkeletalBodySetup* AssociatedBodySetupPtr = nullptr; // 포인터로 변경
    if (CurrentPhysicsAsset)
    {
        for (int32 i = 0; i < CurrentPhysicsAsset->SkeletalBodySetups.Num(); ++i)
        {
            if (CurrentPhysicsAsset->SkeletalBodySetups[i] && CurrentPhysicsAsset->SkeletalBodySetups[i]->BoneName == CurrentBoneName)
            {
                AssociatedBodySetupPtr = CurrentPhysicsAsset->SkeletalBodySetups[i]; // 객체 포인터 저장
                break;
            }
        }
    }

    // 현재 본과 관련된 Constraint 찾기
    TArray<UPhysicsConstraintTemplate*> AssociatedConstraintPtrs; // 포인터 배열로 변경
    if (CurrentPhysicsAsset)
    {
        for (int32 i = 0; i < CurrentPhysicsAsset->ConstraintTemplates.Num(); ++i)
        {
            UPhysicsConstraintTemplate* ConstraintTemplatePtr = CurrentPhysicsAsset->ConstraintTemplates[i];
            if (ConstraintTemplatePtr)
            {
                const FConstraintInstance& CI = ConstraintTemplatePtr->DefaultInstance;
                if (CI.ConstraintBone1 == CurrentBoneName || CI.ConstraintBone2 == CurrentBoneName)
                {
                    AssociatedConstraintPtrs.Add(ConstraintTemplatePtr); // 객체 포인터 저장
                }
            }
        }
    }

    // 자식 본, BodySetup, Constraint가 있는지 확인하여 Leaf 노드 여부 결정
    bool bHasChildren = false;
    for (int32 i = 0; i < RefSkeleton.GetRawBoneNum(); ++i)
    {
        if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == BoneIndex)
        {
            bHasChildren = true;
            break;
        }
    }
    if (!bHasChildren && !AssociatedBodySetupPtr && AssociatedConstraintPtrs.IsEmpty())
    {
        NodeFlags |= ImGuiTreeNodeFlags_Leaf;
        NodeFlags &= ~ImGuiTreeNodeFlags_OpenOnArrow;
    }

    // 선택 상태 반영 (본 자체 선택)
    if (SelectedBoneName == CurrentBoneName && SelectedBodySetup == nullptr && SelectedConstraintTemplate == nullptr)
    {
        NodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    // 아이콘 표시 (본)
    if (BoneIconSRV) ImGui::Image((ImTextureID)BoneIconSRV, ImVec2(16, 16));
    ImGui::SameLine();

    bool bNodeOpen = ImGui::TreeNodeEx(*BoneLabel, NodeFlags);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        SelectedBoneName = CurrentBoneName;
        SelectedBodySetup = nullptr; // 본 자체를 선택
        SelectedConstraintTemplate = nullptr;
    }

    if (bNodeOpen)
    {
        // 1. 연결된 BodySetup 표시
        if (AssociatedBodySetupPtr) // 포인터로 확인
        {
            ImGui::PushID(GetUniqueNodeID()); // BodySetup 노드용 ID
            FString BodyLabel = FString::Printf(TEXT("%s (Body)"), *AssociatedBodySetupPtr->BoneName.ToString());
            ImGuiTreeNodeFlags BodyNodeFlags = ImGuiTreeNodeFlags_Leaf;

            // 선택 상태 반영 (BodySetup)
            if (SelectedBodySetup == AssociatedBodySetupPtr) // 포인터 비교
            {
                BodyNodeFlags |= ImGuiTreeNodeFlags_Selected;
            }

            if (BodyIconSRV) ImGui::Image((ImTextureID)BodyIconSRV, ImVec2(16, 16));
            ImGui::SameLine();

            bool bBodyNodeOpen = ImGui::TreeNodeEx(*BodyLabel, BodyNodeFlags);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                SelectedBodySetup = AssociatedBodySetupPtr; // 객체 포인터 할당
                SelectedConstraintTemplate = nullptr;
                SelectedBoneName = AssociatedBodySetupPtr->BoneName;
            }
            if (bBodyNodeOpen)
            {
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        // 2. 연결된 Constraints 표시
        for (UPhysicsConstraintTemplate* ConstraintTemplatePtr : AssociatedConstraintPtrs) // 포인터로 순회
        {
            ImGui::PushID(GetUniqueNodeID()); // Constraint 노드용 ID
            const FConstraintInstance& CI = ConstraintTemplatePtr->DefaultInstance;
            FString ConstraintLabel = FString::Printf(TEXT("Constraint: %s - %s"), *CI.ConstraintBone1.ToString(), *CI.ConstraintBone2.ToString());
            if (CI.JointName != NAME_None)
            {
                ConstraintLabel = FString::Printf(TEXT("%s (Constraint)"), *CI.JointName.ToString());
            }

            ImGuiTreeNodeFlags ConstraintNodeFlags = ImGuiTreeNodeFlags_Leaf;

            // 선택 상태 반영 (Constraint)
            if (SelectedConstraintTemplate == ConstraintTemplatePtr) // 포인터 비교
            {
                ConstraintNodeFlags |= ImGuiTreeNodeFlags_Selected;
            }

            if (ConstraintIconSRV) ImGui::Image((ImTextureID)ConstraintIconSRV, ImVec2(16, 16));
            ImGui::SameLine();

            bool bConstraintNodeOpen = ImGui::TreeNodeEx(*ConstraintLabel, ConstraintNodeFlags);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                SelectedConstraintTemplate = ConstraintTemplatePtr; // 객체 포인터 할당
                SelectedBodySetup = nullptr;
                SelectedBoneName = NAME_None;
            }
            if (bConstraintNodeOpen)
            {
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        // 3. 자식 본들 재귀적으로 렌더링
        for (int32 i = 0; i < RefSkeleton.GetRawBoneNum(); ++i)
        {
            if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == BoneIndex)
            {
                RenderBoneNodeRecursive(RefSkeleton, i);
            }
        }
        ImGui::TreePop(); // 현재 본 노드 닫기
    }
    ImGui::PopID(); // 현재 본 노드 ID 팝
}

void PhysicsAssetEditorPanel::LoadIcons()
{
}

/*
// 아이콘 로드 함수 예시 (실제 구현 필요)
void PhysicsAssetEditorPanel::LoadIcons()
{
    // BoneIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icons/Bone_Icon.png")->TextureSRV;
    // BodyIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icons/Body_Icon.png")->TextureSRV;
    // ConstraintIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Editor/Icons/Constraint_Icon.png")->TextureSRV;
}
*/
