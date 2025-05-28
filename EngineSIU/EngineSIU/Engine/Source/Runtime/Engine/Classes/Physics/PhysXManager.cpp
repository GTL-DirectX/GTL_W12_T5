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

    CpuDispatcher = PxDefaultCpuDispatcherCreate(4); // 4개 스레드
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
    // 축 맞춰주기 위함.
    SceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

    SceneDesc.cpuDispatcher = CpuDispatcher;
    SceneDesc.filterShader = NoCollisionSelfFilterShader;
    SceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS; // 활성 액터만 업데이트
    SceneDesc.flags |= PxSceneFlag::eENABLE_PCM; // Contact Preprocessing, 충돌 사전 처리 활성화
    SceneDesc.flags |= PxSceneFlag::eENABLE_CCD; // Continuous Collision Detection 활성화

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

void FPhysXManager::ClearPxScene(physx::PxScene* Scene)
{
    if (!Scene)
    {
        return;
    }

    // 쓰기 잠금 (씬 수정 시 필요)
    // Scene->lockWrite(); // PhysX 3.x
    // PhysX 4.x 이상에서는 simulate/fetchResults 외부에서의 수정은 일반적으로 안전하지만,
    // 명시적인 동기화가 필요하다면 적절한 락 사용 고려.
    // 여기서는 간단히 액터 제거만 수행.

    const physx::PxU32 MaxActorsToFetch = 2048; // 한 번에 가져올 최대 액터 수 (필요에 따라 조절)
    physx::PxActor* UserBuffer[MaxActorsToFetch];

    physx::PxU32 NbActors = 0;
    do
    {
        // PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC 등을 사용하여 특정 타입만 가져올 수도 있음
        NbActors = Scene->getActors(physx::PxActorTypeFlag::eRIGID_STATIC | physx::PxActorTypeFlag::eRIGID_DYNAMIC, UserBuffer, MaxActorsToFetch);
        for (physx::PxU32 i = 0; i < NbActors; ++i)
        {
            physx::PxActor* Actor = UserBuffer[i];
            Scene->removeActor(*Actor); // 씬에서 액터 제거
            // Actor->release(); // 중요: 액터를 씬에서 제거한 후, 해당 액터 객체 자체도 release 해야 메모리 누수가 발생하지 않습니다.
            // 단, 이 액터가 다른 곳에서 여전히 참조되고 있다면 release 시점을 조절해야 합니다.
            // 만약 이 액터들이 PhysicsViewer 전용으로 생성된 임시 객체라면 여기서 release하는 것이 맞습니다.
        }
    } while (NbActors == MaxActorsToFetch); // 모든 액터를 가져올 때까지 반복

    // Scene->unlockWrite(); // PhysX 3.x
}
