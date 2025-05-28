#pragma once
#include "Player.h"
class UInputComponent;
class ADefaultPlayer : public APlayer
{
    DECLARE_CLASS(ADefaultPlayer, APlayer)
public:
    ADefaultPlayer() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void Tick(float DeltaTime) override;    
    virtual void PostSpawnInitialize() override;
    virtual void SetUpPlayerInputComponent(UInputComponent* InInputComponent) override;
private:
    void MoveForward(float DeltaTime);
    void MoveRight(float DeltaTime);
    void MoveUp(float DeltaTime);
private:
    float MoveSpeed = 100.0f; // 이동 속도
};

