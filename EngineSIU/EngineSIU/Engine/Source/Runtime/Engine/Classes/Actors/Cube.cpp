#include "Cube.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Physics/BodySetup.h"

ACube::ACube()
{
    StaticMeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Reference/Reference.obj"));

    if (!StaticMeshComponent->GetBodySetup())
    {
        StaticMeshComponent->GetStaticMesh()->SetBodySetup(FObjectFactory::ConstructObject<UBodySetup>(this));
        StaticMeshComponent->GetBodySetup()->AddBoxElem(FKBoxElem(100.5f, 0.5f, 100.5f));
        StaticMeshComponent->GetBodySetup()->AddSphereElem(FKSphereElem(1.f));
    }
        StaticMeshComponent->GetBodySetup()->AggGeom.SphylElems.Add(FKSphylElem(0.5f, 1.f));
    
}

void ACube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //SetActorRotation(GetActorRotation() + FRotator(0, 0, 1));

}
