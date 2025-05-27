#pragma once
#include "ActorComponent.h"
#include "SceneComponent.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class UPostProcessVolume : public USceneComponent
{
    DECLARE_CLASS(UPostProcessVolume, USceneComponent)

public:
    UPostProcessVolume() = default;
    virtual ~UPostProcessVolume() override = default;

    //초점 거리
    UPROPERTY(EPropertyFlags::EditAnywhere, float, DepthOfFieldFocalDistance, = 0)
    //초점 거리 범위
    UPROPERTY(EPropertyFlags::EditAnywhere, float, DepthOfFieldSensorWidth, = 24.576f)
    //조리개 F값
    UPROPERTY(EPropertyFlags::EditAnywhere, float, DepthOfFieldFstop, = 4.f)
    //조리개 최소 F값
    UPROPERTY(EPropertyFlags::EditAnywhere, float, DepthOfFieldMinFstop, = 1.2f) 
    
};
