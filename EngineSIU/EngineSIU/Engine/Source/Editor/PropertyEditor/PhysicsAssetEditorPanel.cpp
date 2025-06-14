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
#include "Physics/PhysXManager.h"
#include "UnrealEd/ImGuiWidget.h"
#include "World/SimulationViewerWorld.h"

PhysicsAssetEditorPanel::PhysicsAssetEditorPanel()
{
    // 이 패널이 지원하는 월드 타입을 설정할 수 있습니다.
    SetSupportedWorldTypes(EWorldTypeBitFlag::PhysicsViewer | EWorldTypeBitFlag::SimulationViewer);
    
}

PhysicsAssetEditorPanel::~PhysicsAssetEditorPanel()
{
}

void PhysicsAssetEditorPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    if (BoneIconSRV == nullptr || BodyIconSRV == nullptr) {
        LoadIcons();
    }

    USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Engine->GetSelectedComponent());

    if (SkeletalMeshComponent and SkeletalMeshComponent->GetSkeletalMeshAsset() != CurrentSkeletalMesh)
    {
        SetSkeletalMesh(SkeletalMeshComponent->GetSkeletalMeshAsset());
    }
    
    // TODO 월드 틱으로 옮겨야 할 듯
    if (Engine->ActiveWorld->WorldType == EWorldType::PhysicsViewer and CurrentPhysicsAsset != nullptr)
    {
        physx::PxScene* ViewerPxScene = Engine->ActiveWorld->GetPhysicsScene(); // 이 월드의 PxScene 가져오기
        // 1. 기존 PhysicsViewer PxScene의 모든 액터 제거
        FPhysXManager::Get().ClearPxScene(ViewerPxScene); // 위에서 정의한 함수 사용

        if (SkeletalMeshComponent)
            SkeletalMeshComponent->InitPhysicsBodies();
    }

    
    // if (!Engine || !CurrentPhysicsAsset)
    // {
    //     // 엔진이나 피직스 에셋이 없으면 아무것도 렌더링하지 않음
    //     // 또는 "피직스 에셋을 선택해주세요" 같은 메시지 표시 가능
    //     ImGui::Begin("Physics Asset Editor");
    //     ImGui::Text("No Physics Asset selected or loaded.");
    //     ImGui::End();
    //     return;
    // }

    if (CurrentPhysicsAsset)
    {
        CurrentPhysicsAsset->UpdateBodySetupIndexMap();
    }


    // --- 버튼들을 위한 공간 확보 및 스타일 ---
    const float ButtonHeight = 60.0f;
    const float ButtonPanelPadding = 5.0f; // 버튼과 창 가장자리 사이의 간격
    const float TopButtonY = ButtonPanelPadding;
    const float BottomButtonY = Height - ButtonHeight - ButtonPanelPadding;

    // --- 좌상단 저장 버튼 ---
    ImGui::SetNextWindowPos(ImVec2(Width - 170 - ButtonPanelPadding, TopButtonY), ImGuiCond_Always); // 버튼 너비만큼 왼쪽으로 이동
    ImGui::SetNextWindowSize(ImVec2(80, ButtonHeight), ImGuiCond_Always); // 버튼 크기에 맞게 조절
    ImGui::Begin("SaveAssetButton", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
    if (ImGui::Button("Save", ImVec2(-1, -1))) // -1은 사용 가능한 전체 너비/높이를 의미
    {
        if (CurrentPhysicsAsset)
        {
            //스켈레탈 메쉬 이름으로 저장
            UAssetManager::Get().SavePhysicsAssets(CurrentPhysicsAsset, CurrentPhysicsAsset->GetName());
        }
    }
    ImGui::End();

    // --- 우상단 시뮬레이션 버튼 ---
    ImGui::SetNextWindowPos(ImVec2(Width - 90 - ButtonPanelPadding, TopButtonY), ImGuiCond_Always); // 버튼 너비만큼 왼쪽으로 이동
    ImGui::SetNextWindowSize(ImVec2(100, ButtonHeight), ImGuiCond_Always); // 버튼 크기에 맞게 조절 (텍스트 길이에 따라)
    ImGui::Begin("SimulateButton", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
    // 시뮬레이션 상태에 따라 버튼 텍스트 변경 (예시)

    
    bool bIsSimulating;
    if (Engine->ActiveWorld->WorldType == EWorldType::SimulationViewer)
    {
        bIsSimulating = true;
    }
    else
    {
        bIsSimulating = false; // 현재 시뮬레이션 상태를 확인하는 로직 필요
    }

    if (ImGui::Button(bIsSimulating ? "Stop Sim" : "Simulate", ImVec2(-1, -1)))
    {
        if (bIsSimulating)
        {
            Engine->EndSimulationViewer(); // 엔진에 이런 함수가 있다고 가정
        }
        else
        {
            Engine->StartSimulationViewer(); // 엔진에 이런 함수가 있다고 가정
        }
    }
    ImGui::End();

    // --- 스켈레톤 트리 패널 렌더링 ---
    const float PanelStartY = TopButtonY + ButtonHeight + ButtonPanelPadding;
    const float PanelAvailableHeight = Height - PanelStartY - (ButtonHeight + ButtonPanelPadding * 2); // 상단 버튼 영역과 하단 버튼 영역 제외

    const float TreePanelWidth = Width * 0.25f;
    const float TreePanelHeight = PanelAvailableHeight;
    const float TreePanelPosX = 0; // 화면 왼쪽 시작

    ImGui::SetNextWindowPos(ImVec2(TreePanelPosX, PanelStartY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(TreePanelWidth, TreePanelHeight), ImGuiCond_Always);

    constexpr ImGuiWindowFlags TreePanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;

    ImGui::Begin("Physics Hierarchy", nullptr, TreePanelFlags);
    {
        ImGuiNodeIdCounter = 0;
        RenderSkeletonTree(); // 이 함수 내부에서 SelectedBodySetup, SelectedConstraintTemplate이 업데이트됨
    }
    ImGui::End();

    // --- 디테일 패널 렌더링 ---
    const float DetailsPanelWidth = Width * 0.25f; // 디테일 패널 너비를 전체의 35% 정도로 설정 (조절 가능)
    const float DetailsPanelPosX = Width - DetailsPanelWidth; // 화면 오른쪽 끝에서 DetailsPanelWidth만큼 떨어진 위치
    const float DetailsPanelHeight = PanelAvailableHeight;

    ImGui::SetNextWindowPos(ImVec2(DetailsPanelPosX, PanelStartY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(DetailsPanelWidth, DetailsPanelHeight), ImGuiCond_Always);

    constexpr ImGuiWindowFlags DetailsPanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::Begin("Details", nullptr, DetailsPanelFlags);
    {
        RenderDetailsPanel();
    }
    ImGui::End();

    // --- 우하단 나가기 버튼 ---
    ImGui::SetNextWindowPos(ImVec2(Width - 80 - ButtonPanelPadding, BottomButtonY), ImGuiCond_Always); // 버튼 너비만큼 왼쪽으로 이동
    ImGui::SetNextWindowSize(ImVec2(80, ButtonHeight), ImGuiCond_Always);
    ImGui::Begin("ExitButton", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
    if (ImGui::Button("Exit", ImVec2(-1, -1)))
    {
        // 나가기 로직 (예: 에디터 패널 닫기 또는 이전 뷰로 돌아가기)
        // UEditorEngine에 해당 기능이 있다면 호출
        Engine->EndPhysicsViewer(); // 이런 함수가 있다고 가정
    }
    ImGui::End();

    

    if (SelectedBoneName != NAME_None)
    {
        //// SelectBoneIndex 처리
        const FReferenceSkeleton& RefSkeleton = CurrentSkeletalMesh->GetSkeleton()->GetReferenceSkeleton();

        for (int32 i = 0; i < RefSkeleton.GetRawBoneNum(); ++i)
        {
            if (RefSkeleton.RawRefBoneInfo[i].Name == SelectedBoneName) // 루트 본인 경우
            {
                if (UPhysicsViewerWorld* PhysicsViewerWorld = Cast<UPhysicsViewerWorld>(Engine->ActiveWorld))
                {
                    PhysicsViewerWorld->SelectBoneIndex = i; // PhysicsViewerWorld에 선택된 본 인덱스 설정
                }
                else if (USimulationViewerWorld* SimulationViewerWorld = Cast<USimulationViewerWorld>(Engine->ActiveWorld))
                {
                    SimulationViewerWorld->SelectBoneIndex = i; // SkeletalViewerWorld에 선택된 본 인덱스 설정
                }
                break; // 찾았으면 루프 종료
            }
        }
    }

}

void PhysicsAssetEditorPanel::SetSkeletalMesh(USkeletalMesh* InSkeletalMesh)
{
    CurrentSkeletalMesh = InSkeletalMesh;
    if (!CurrentSkeletalMesh)
    {
        return;
    }
    
    if (CurrentSkeletalMesh->PhysicsAsset == nullptr)
    {
        CurrentSkeletalMesh->CreateOrBindPhysicsAsset();
    }
    SetPhysicsAsset(CurrentSkeletalMesh->PhysicsAsset);
    
    SelectedBoneName = NAME_None;
    
    SelectedBodySetup = nullptr; // 선택된 BodySetup 객체
    SelectedConstraintTemplate = nullptr; // 선택된 ConstraintTemplate 객체
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
    SelectedBodySetup = nullptr; // 선택된 BodySetup 객체
    SelectedConstraintTemplate = nullptr; // 선택된 ConstraintTemplate 객체

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

        // 삭제 버튼 추가

        ImGui::SameLine(); // 필요하다면 다른 버튼과 같은 줄에 배치
        
        RenderObjectDetails(SelectedBodySetup); // UBodySetup (및 USkeletalBodySetup)의 프로퍼티 표시
        RenderConstraintCreationUI(SelectedBodySetup->BoneName, TEXT("FromSelectedBodySetup")); // UiContextId 전달

        if (SelectedBodySetup && ImGui::Button(*FString::Printf(TEXT("Delete BodySetup: %s"), *SelectedBodySetup->BoneName.ToString())))
        {
            HandleDeleteSelectedBodySetup(); // 삭제 핸들러 호출
            return; // 중요: 삭제 후에는 이 프레임에서 더 이상 이 객체에 접근하지 않도록 함
        }
    }
    else if (SelectedConstraintTemplate)
    {
        const FConstraintInstance& CI = SelectedConstraintTemplate->DefaultInstance;
        FString ConstraintName = CI.JointName != NAME_None ? CI.JointName.ToString() : FString::Printf(TEXT("%s - %s"), *CI.ConstraintBone1.ToString(), *CI.ConstraintBone2.ToString());

        ImGui::SeparatorText(*FString::Printf(TEXT("Constraint: %s"), *ConstraintName));

        // 삭제 버튼 추가

        ImGui::SameLine();
        // (만약 다른 버튼이 있다면) ImGui::Spacing();
        
        RenderObjectDetails(SelectedConstraintTemplate); // UPhysicsConstraintTemplate의 프로퍼티

        if (ImGui::Button(*FString::Printf(TEXT("Delete Constraint: %s"), *ConstraintName)))
        {
            HandleDeleteSelectedConstraintTemplate(); // 삭제 핸들러 호출
            // 삭제 후에는 SelectedConstraintTemplate이 유효하지 않으므로, 바로 리턴
            return;
        }
    }
    else if (SelectedBoneName != NAME_None)
    {
        ImGui::SeparatorText(*FString::Printf(TEXT("Bone: %s"), *SelectedBoneName.ToString()));
        
        // 현재 선택된 본에 이미 BodySetup이 있는지 확인
        bool bHasExistingBodySetup = false;
        USkeletalBodySetup* ExistingBodySetup = nullptr;
        if (CurrentPhysicsAsset)
        {
            for (USkeletalBodySetup* BodySetup : CurrentPhysicsAsset->SkeletalBodySetups)
            {
                if (BodySetup && BodySetup->BoneName == SelectedBoneName)
                {
                    bHasExistingBodySetup = true;
                    ExistingBodySetup = BodySetup; // 기존 BodySetup을 참조
                    break;
                }
            }
        }

        if (bHasExistingBodySetup && ExistingBodySetup)
        {
            // 이미 BodySetup이 존재하면, 해당 BodySetup의 디테일을 표시하도록 유도하거나,
            // 선택을 해당 BodySetup으로 변경할 수 있습니다.
            ImGui::Text("This bone already has a BodySetup.");
            if (ImGui::Button(*FString::Printf(TEXT("Select BodySetup for %s"), *SelectedBoneName.ToString())))
            {
                SelectedBodySetup = ExistingBodySetup; // 선택을 기존 BodySetup으로 변경
                // SelectedBoneName은 유지하거나, BodySetup 선택 시 어떻게 할지 정책에 따라 결정
            }
            RenderConstraintCreationUI(SelectedBoneName, TEXT("FromSelectedBoneWithBodySetup")); // UiContextId 전달
        }
        else
        {
            // BodySetup이 없는 경우, 생성 버튼 표시
            FString ButtonLabel = FString::Printf(TEXT("Create BodySetup for %s"), *SelectedBoneName.ToString());
            if (ImGui::Button(*ButtonLabel))
            {
                if (CurrentPhysicsAsset) // CurrentPhysicsAsset이 유효한지 다시 한번 확인
                {
                    // 새 USkeletalBodySetup 객체 생성
                    // Outer를 CurrentPhysicsAsset으로 지정하여 소유 관계를 명확히 합니다.
                    USkeletalBodySetup* NewBodySetup = FObjectFactory::ConstructObject<USkeletalBodySetup>(CurrentPhysicsAsset);
                    if (NewBodySetup)
                    {
                        NewBodySetup->BoneName = SelectedBoneName;
                        CurrentPhysicsAsset->SkeletalBodySetups.Add(NewBodySetup);
                        CurrentPhysicsAsset->UpdateBodySetupIndexMap(); // BodySetup 추가 후 맵 업데이트

                        // 새로 생성된 BodySetup을 바로 선택 상태로 만들 수 있습니다.
                        SelectedBodySetup = NewBodySetup;
                        SelectedBoneName = NAME_None; // BodySetup이 선택되었으므로 본 자체 선택은 해제 (선택 사항)

                        UE_LOG(ELogLevel::Display, TEXT("Created BodySetup for bone: %s"), *SelectedBoneName.ToString());
                    }
                    else
                    {
                        UE_LOG(ELogLevel::Error, TEXT("Failed to construct USkeletalBodySetup for bone: %s"), *SelectedBoneName.ToString());
                    }
                }
            }
        }

        
        
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
    BoneIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/Bone_16x.PNG")->TextureSRV;
    BodyIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/RigidBody_16x.PNG")->TextureSRV;
    ConstraintIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/Constraint.PNG")->TextureSRV;

}


 void PhysicsAssetEditorPanel::RenderConstraintCreationUI(FName FirstBoneName, const FString& UiContextId)
{
    if (FirstBoneName == NAME_None || !CurrentPhysicsAsset)
    {
        return;
    }

    ImGui::SeparatorText(*FString::Printf(TEXT("Create Constraint from %s to..."), *FirstBoneName.ToString()));

    // 각 UI 컨텍스트마다 고유한 static 변수를 사용하기 위해 ID를 활용하거나,
    // 멤버 변수로 SecondBoneToConstrain을 컨텍스트별로 관리해야 할 수 있습니다.
    // 여기서는 간단하게 UiContextId를 사용하여 ImGui ID를 다르게 합니다.
    // 더 나은 방법은 멤버 변수로 각 컨텍스트의 선택 상태를 저장하는 것입니다.
    // 예: static TMap<FString, FName> ContextualSecondBoneSelection;
    // 여기서는 설명을 위해 간단한 static 변수를 사용하되, ID로 구분합니다.

    ImGui::PushID(*UiContextId); // ID 푸시

    static FName SecondBoneToConstrain = NAME_None;
    FString ComboLabel = FString::Printf(TEXT("Connect to Body##%s"), *UiContextId); // 고유 레이블
    FString SecondBoneComboPreview = SecondBoneToConstrain == NAME_None ? TEXT("Select Second Body...") : SecondBoneToConstrain.ToString();

    if (ImGui::BeginCombo(*ComboLabel, *SecondBoneComboPreview))
    {
        for (USkeletalBodySetup* OtherBodySetup : CurrentPhysicsAsset->SkeletalBodySetups)
        {
            // 첫 번째 본과 다른 BodySetup만 선택 가능하도록
            if (OtherBodySetup && OtherBodySetup->BoneName != FirstBoneName)
            {
                if (ImGui::Selectable(*OtherBodySetup->BoneName.ToString(), SecondBoneToConstrain == OtherBodySetup->BoneName))
                {
                    SecondBoneToConstrain = OtherBodySetup->BoneName;
                }
            }
        }
        ImGui::EndCombo();
    }

    if (SecondBoneToConstrain != NAME_None)
    {
        FString ButtonLabel = FString::Printf(TEXT("Create Constraint (%s - %s)##%s"), *FirstBoneName.ToString(), *SecondBoneToConstrain.ToString(), *UiContextId);
        if (ImGui::Button(*ButtonLabel))
        {
            // 이미 두 본 사이에 제약 조건이 있는지 확인
            bool bConstraintExists = false;
            for (UPhysicsConstraintTemplate* ExistingConstraint : CurrentPhysicsAsset->ConstraintTemplates)
            {
                if (ExistingConstraint)
                {
                    const FConstraintInstance& CI = ExistingConstraint->DefaultInstance;
                    if ((CI.ConstraintBone1 == FirstBoneName && CI.ConstraintBone2 == SecondBoneToConstrain) ||
                        (CI.ConstraintBone1 == SecondBoneToConstrain && CI.ConstraintBone2 == FirstBoneName))
                    {
                        bConstraintExists = true;
                        UE_LOG(ELogLevel::Warning, TEXT("Constraint already exists between %s and %s."), *FirstBoneName.ToString(), *SecondBoneToConstrain.ToString());
                        break;
                    }
                }
            }

            if (!bConstraintExists)
            {
                UPhysicsConstraintTemplate* NewConstraint = FObjectFactory::ConstructObject<UPhysicsConstraintTemplate>(CurrentPhysicsAsset);
                if (NewConstraint)
                {
                    NewConstraint->DefaultInstance.ConstraintBone1 = FirstBoneName;
                    NewConstraint->DefaultInstance.ConstraintBone2 = SecondBoneToConstrain;
                    NewConstraint->DefaultInstance.JointName = FName(*FString::Printf(TEXT("[%s -> %s] Constraint"), *FirstBoneName.ToString(), *SecondBoneToConstrain.ToString()));

                    CurrentPhysicsAsset->ConstraintTemplates.Add(NewConstraint);

                    SelectedConstraintTemplate = NewConstraint;
                    SelectedBodySetup = nullptr;
                    SelectedBoneName = NAME_None;
                    SecondBoneToConstrain = NAME_None; // 중요: 여기서 초기화하면 다음 프레임에 콤보박스가 초기화됨.
                                                       // UI 상태 유지를 위해 호출부에서 적절히 관리하거나,
                                                       // 성공적으로 생성 후 명시적으로 초기화 필요.

                    UE_LOG(ELogLevel::Display, TEXT("Created Constraint between %s and %s."), *NewConstraint->DefaultInstance.ConstraintBone1.ToString(), *NewConstraint->DefaultInstance.ConstraintBone2.ToString());
                }
                else
                {
                    UE_LOG(ELogLevel::Error, TEXT("Failed to construct UPhysicsConstraintTemplate."));
                }
            }
            // 제약 조건 생성 시도 후에는 SecondBoneToConstrain 선택을 초기화하여
            // 다음 번 UI 표시 시 "Select Second Body..."로 나타나도록 합니다.
            SecondBoneToConstrain = NAME_None;
        }
    }
    ImGui::PopID(); // ID 팝
}

void PhysicsAssetEditorPanel::HandleDeleteSelectedBodySetup()
{
    if (CurrentPhysicsAsset && SelectedBodySetup)
    {
        FName BodySetupNameToDelete = SelectedBodySetup->BoneName; // 로그용

        // 1. CurrentPhysicsAsset->SkeletalBodySetups 배열에서 해당 BodySetup 제거
        CurrentPhysicsAsset->SkeletalBodySetups.Remove(SelectedBodySetup);

        // 2. 이 BodySetup을 참조하는 모든 Constraint도 함께 제거해야 함
        for (int32 i = CurrentPhysicsAsset->ConstraintTemplates.Num() - 1; i >= 0; --i)
        {
            UPhysicsConstraintTemplate* Constraint = CurrentPhysicsAsset->ConstraintTemplates[i];
            if (Constraint)
            {
                const FConstraintInstance& ConstraintInstance = Constraint->DefaultInstance;
                if (ConstraintInstance.ConstraintBone1 == SelectedBodySetup->BoneName || ConstraintInstance.ConstraintBone2 == SelectedBodySetup->BoneName)
                {
                    // 만약 현재 선택된 Constraint가 삭제 대상 Constraint라면, 선택도 해제
                    if (SelectedConstraintTemplate == Constraint)
                    {
                        SelectedConstraintTemplate = nullptr;
                    }
                    CurrentPhysicsAsset->ConstraintTemplates.RemoveAt(i);
                    UE_LOG(ELogLevel::Display, TEXT("Automatically deleted Constraint referencing BodySetup %s: %s"), *BodySetupNameToDelete.ToString(), *(ConstraintInstance.JointName != NAME_None ? ConstraintInstance.JointName.ToString() : FString::Printf(TEXT("%s-%s"), *ConstraintInstance.ConstraintBone1.ToString(), *ConstraintInstance.ConstraintBone2.ToString())));
                }
            }
        }

        // 3. 내부 인덱스 맵 업데이트 (필요하다면)
        CurrentPhysicsAsset->UpdateBodySetupIndexMap();

        UE_LOG(ELogLevel::Display, TEXT("Deleted BodySetup: %s"), *BodySetupNameToDelete.ToString());
        
        // 4. 선택 상태 초기화
        SelectedBodySetup = nullptr;
        // SelectedBoneName = NAME_None; // 본 선택도 해제할지 여부 결정
        
    }
}

void PhysicsAssetEditorPanel::HandleDeleteSelectedConstraintTemplate()
{
    if (CurrentPhysicsAsset && SelectedConstraintTemplate)
    {
        FString ConstraintNameToDelete = SelectedConstraintTemplate->DefaultInstance.JointName != NAME_None ?
                                         SelectedConstraintTemplate->DefaultInstance.JointName.ToString() :
                                         FString::Printf(TEXT("%s - %s"), *SelectedConstraintTemplate->DefaultInstance.ConstraintBone1.ToString(), *SelectedConstraintTemplate->DefaultInstance.ConstraintBone2.ToString());


        // 1. CurrentPhysicsAsset->ConstraintTemplates 배열에서 해당 ConstraintTemplate 제거
        CurrentPhysicsAsset->ConstraintTemplates.Remove(SelectedConstraintTemplate);


        UE_LOG(ELogLevel::Display, TEXT("Deleted ConstraintTemplate: %s"), *ConstraintNameToDelete);

        // 3. 선택 상태 초기화
        SelectedConstraintTemplate = nullptr;
    }
}

