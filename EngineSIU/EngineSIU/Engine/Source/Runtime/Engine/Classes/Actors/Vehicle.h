#pragma once
#include "Player.h"
class UCameraComponent;
class UStaticMeshComponent;
class AVehicle : public APlayer
{
    DECLARE_CLASS(AVehicle, APlayer)
public: 
    AVehicle();
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetUpPlayerInputComponent(UInputComponent* InInputComponent) override;
private:
    UPROPERTY(EditAnywhere|EditInline,
    UCameraComponent*, CameraComponent, = nullptr)
    UPROPERTY(EditAnywhere|EditInline,
        UStaticMeshComponent*, StaticMeshCompoent, = nullptr)
private:
    void MoveForward(float DeltaTime);
    void MoveRight(float DeltaTime);
    void MoveUp(float DeltaTime);
    float MoveSpeed = 100.0f; // 이동 속도
};

