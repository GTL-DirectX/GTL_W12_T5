#include "FPhysXManager.h"

FPhysXManager& FPhysXManager::Get()
{
    static FPhysXManager Instance;
    return Instance;
}
