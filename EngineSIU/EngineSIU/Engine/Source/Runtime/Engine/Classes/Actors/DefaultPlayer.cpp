#include "DefaultPlayer.h"
#include "Components/InputComponent.h"
#include "GameFrameWork/PlayerController.h"
#include "UnrealClient.h"
#include "World/World.h"
#include "Engine/Engine.h"
UObject* ADefaultPlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    return NewActor;
}

void ADefaultPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADefaultPlayer::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
}

void ADefaultPlayer::SetUpPlayerInputComponent(UInputComponent* InInputComponent)
{
    InInputComponent->BindAction("W", [this](float DeltaTime) { MoveForward(DeltaTime); });
    InInputComponent->BindAction("S", [this](float DeltaTime) { MoveForward(-DeltaTime); });
    InInputComponent->BindAction("A", [this](float DeltaTime) { MoveRight(-DeltaTime); });
    InInputComponent->BindAction("D", [this](float DeltaTime) { MoveRight(DeltaTime); });
    InInputComponent->BindAction("Q", [this](float DeltaTime) { MoveUp(DeltaTime); });
    InInputComponent->BindAction("E", [this](float DeltaTime) { MoveUp(-DeltaTime); });

    InInputComponent->BindAxis("Turn", [this](float DeltaTime) { RotateYaw(DeltaTime); });
    InInputComponent->BindAxis("LookUp", [this](float DeltaTime) { RotatePitch(DeltaTime); });    
}

void ADefaultPlayer::MoveForward(float DeltaTime)
{
    FVector Delta = GetActorForwardVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void ADefaultPlayer::MoveRight(float DeltaTime)
{
    FVector Delta = GetActorRightVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void ADefaultPlayer::MoveUp(float DeltaTime)
{
    FVector Delta = GetActorUpVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void ADefaultPlayer::RotateYaw(float DeltaTime)
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += DeltaTime * RotationSpeed; // Yaw 회전 속도
    SetActorRotation(NewRotation);
}

void ADefaultPlayer::RotatePitch(float DeltaTime)
{
    FRotator NewRotation = GetActorRotation();
    NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - DeltaTime*RotationSpeed, -89.0f, 89.0f);
    SetActorRotation(NewRotation);
}

