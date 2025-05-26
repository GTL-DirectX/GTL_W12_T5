#include "PhysXManager.h"

#include "UserInterface/Console.h"

FPhysXManager::FPhysXManager()
{
    InitPhysX();
}

FPhysXManager::~FPhysXManager()
{
    ShutdonwPhysX();
}

FPhysXManager& FPhysXManager::Get()
{
    static FPhysXManager Instance;
    return Instance;
}

void FPhysXManager::InitPhysX()
{
    PxFoundation* Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, Allocator, ErrorCallback);
    if (!Foundation)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create PhysX Foundation!"));
        return;
    }
    PxTolerancesScale Scale;
    Scale.length = 1.0f;       // 1 unit = 1 cm
    Scale.speed = 980.0f;      // 1g ≈ 980 cm/s²

    Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, Scale, true, nullptr);
    if (!Physics)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create PxPhysics!"));
        return;
    }

    // Cooking 생성 (Convex Mesh, Triangle Mesh 등 cook 용)
    PxCookingParams CookingParams(Scale);
    CookingParams.meshPreprocessParams = PxMeshPreprocessingFlag::eWELD_VERTICES;
    Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *Foundation, CookingParams);
    if (!Cooking)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create PxCooking!"));
        return;
    }

    CpuDispatcher = PxDefaultCpuDispatcherCreate(2); // 2개 스레드
    if (!CpuDispatcher)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create PxDefaultCpuDispatcher!"));
        return;
    }

    // 기본 Material 생성
    DefaultMaterial = Physics->createMaterial(0.5f, 0.5f, 0.6f);
    if (!DefaultMaterial)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create default PxMaterial!"));
        return;
    }

    UE_LOG(ELogLevel::Display, TEXT("PhysX Initialized successfully."));
}

void FPhysXManager::ShutdonwPhysX()
{
    if (DefaultMaterial)
    {
        DefaultMaterial->release();
        DefaultMaterial = nullptr;
    }
    if (CpuDispatcher)
    {
        CpuDispatcher->release();
        CpuDispatcher = nullptr;
    }
    if (Cooking)
    {
        Cooking->release();
        Cooking = nullptr;
    }
    if (Physics)
    {
        Physics->release();
        Physics = nullptr;
    }
    if (Foundation)
    {
        Foundation->release();
        Foundation = nullptr;
    }
}

PxScene* FPhysXManager::CreateScene()
{
    PxSceneDesc SceneDesc(Physics->getTolerancesScale());
    SceneDesc.gravity = PxVec3(0.0f, 0.0f, -9.81f);

    SceneDesc.cpuDispatcher = CpuDispatcher;
    SceneDesc.filterShader = PxDefaultSimulationFilterShader;

    PxScene* NewScene = Physics->createScene(SceneDesc);
    if (!NewScene)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create PxScene!"));
        return nullptr;
    }

    return NewScene;
}
