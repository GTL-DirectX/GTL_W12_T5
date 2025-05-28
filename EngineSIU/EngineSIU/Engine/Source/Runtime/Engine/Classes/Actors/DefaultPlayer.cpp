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
    APlayerController* PC = GEngine->ActiveWorld->GetPlayerController();
    if (PC)
    {
        PC->BindAction("W", [this](float DeltaTime) { MoveForward(DeltaTime); });
        PC->BindAction("S", [this](float DeltaTime) { MoveForward(-DeltaTime); });
        PC->BindAction("A", [this](float DeltaTime) { MoveRight(-DeltaTime); });
        PC->BindAction("D", [this](float DeltaTime) { MoveRight(DeltaTime); });
        PC->BindAction("Q", [this](float DeltaTime) { MoveUp(DeltaTime); });
        PC->BindAction("E", [this](float DeltaTime) { MoveUp(-DeltaTime); });
    }
}

void ADefaultPlayer::SetUpPlayerInputComponent(UInputComponent* InInputComponent)
{
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
