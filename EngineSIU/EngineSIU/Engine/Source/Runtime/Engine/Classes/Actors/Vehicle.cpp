#include "Vehicle.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
AVehicle::AVehicle()
{
}

UObject* AVehicle::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    return NewActor;
}

void AVehicle::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    // 차량 카메라 컴포넌트 생성 및 설정
    StaticMeshCompoent = AddComponent<UStaticMeshComponent>(FName("VehicleMesh"));
    StaticMeshCompoent->SetStaticMesh(UAssetManager::Get().GetStaticMesh(TEXT(L"Contents/Vehicle/SKM_SportsCar.obj")));
    StaticMeshCompoent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f)); // 차량 메쉬 회전 조정
    SetRootComponent(StaticMeshCompoent);
    CameraComponent = AddComponent<UCameraComponent>(FName("VehicleCamera"));
    CameraComponent->SetupAttachment(StaticMeshCompoent);
    CameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 10.f)); // 카메라 위치 조정
    CameraComponent->SetRelativeRotation(FRotator(-45,0, 0)); // 카메라 위치 조정
}

void AVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AVehicle::SetUpPlayerInputComponent(UInputComponent* InInputComponent)
{
    InInputComponent->BindAction("W", [this](float DeltaTime) { MoveForward(DeltaTime); });
    InInputComponent->BindAction("S", [this](float DeltaTime) { MoveForward(-DeltaTime); });
    InInputComponent->BindAction("A", [this](float DeltaTime) { MoveRight(-DeltaTime); });
    InInputComponent->BindAction("D", [this](float DeltaTime) { MoveRight(DeltaTime); });
    InInputComponent->BindAction("Q", [this](float DeltaTime) { MoveUp(DeltaTime); });
    InInputComponent->BindAction("E", [this](float DeltaTime) { MoveUp(-DeltaTime); });
}

void AVehicle::MoveForward(float DeltaTime)
{
    FVector Delta = GetActorForwardVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void AVehicle::MoveRight(float DeltaTime)
{
    FVector Delta = GetActorRightVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void AVehicle::MoveUp(float DeltaTime)
{
    FVector Delta = GetActorUpVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}
