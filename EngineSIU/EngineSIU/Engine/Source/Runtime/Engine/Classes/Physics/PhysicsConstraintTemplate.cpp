#include "PhysicsConstraintTemplate.h"

void UPhysicsConstraintTemplate::Serialize(FArchive& Ar)
{
    UObject::Serialize(Ar);

    Ar << DefaultInstance;
}
