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

    PvdInstance = PxCreatePvd(*Foundation);
    Transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    PvdInstance->connect(*Transport, PxPvdInstrumentationFlag::eALL);

    Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, Scale, true, PvdInstance);
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

// 같은 CompId를 가진 객체끼리 충돌을 방지하는 필터 셰이더.
static PxFilterFlags NoCollisionSelfFilterShader(PxFilterObjectAttributes attributes0,
    PxFilterData filterData0,
    PxFilterObjectAttributes attributes1,
    PxFilterData filterData1,
    PxPairFlags& pairFlags,
    const void* constantBlock,
    PxU32 constantBlockSize)
{
    if (filterData0.word0 != 0 && filterData0.word0 == filterData1.word0)
    {
        return PxFilterFlag::eSUPPRESS;
    }
    pairFlags = PxPairFlag::eCONTACT_DEFAULT;
    return PxFilterFlags();
}

PxScene* FPhysXManager::CreateScene()
{
    PxSceneDesc SceneDesc(Physics->getTolerancesScale());
    SceneDesc.gravity = PxVec3(0.0f, 0.0f, -9.81f);

    SceneDesc.cpuDispatcher = CpuDispatcher;
    SceneDesc.filterShader = NoCollisionSelfFilterShader;

    PxScene* NewScene = Physics->createScene(SceneDesc);
    if (!NewScene)
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create PxScene!"));
        return nullptr;
    }

    PxPvdSceneClient* PvdClient = NewScene->getScenePvdClient();
    if (PvdClient)
    {
        PvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        PvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        PvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    return NewScene;
}

void FPhysXManager::DestroyScene(PxScene* Scene)
{
    if (Scene)
    {
        Scene->release();
        Scene = nullptr;
    }
}
