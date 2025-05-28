#include "SphereActor.h"

#include "UObject/ObjectFactory.h"
#include "Physics/BodySetup.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/FObjLoader.h"

ASphereActor::ASphereActor()
{
    StaticMeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Primitives/SpherePrimitive.obj"));

    if (!StaticMeshComponent->GetBodySetup())
    {
        StaticMeshComponent->GetStaticMesh()->SetBodySetup(FObjectFactory::ConstructObject<UBodySetup>(this));
        StaticMeshComponent->GetBodySetup()->AddSphereElem(FKSphereElem(0.5f));
    }
}
