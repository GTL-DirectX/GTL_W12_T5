#pragma once
#include "BoxElem.h"
#include "ConvexElem.h"
#include "SphereElem.h"
#include "SphylElem.h"

struct FKAggregateGeom
{
    DECLARE_STRUCT(FKAggregateGeom)
    
    UPROPERTY(
        EditAnywhere,
        TArray<FKSphereElem>, SphereElems, {};
    )
    
    UPROPERTY(
        EditAnywhere,
        TArray<FKBoxElem>, BoxElems, {};
    )
    
    UPROPERTY(
        EditAnywhere,
        TArray<FKSphylElem>, SphylElems, {};
    )
    
    UPROPERTY(
        EditAnywhere,
        TArray<FKConvexElem>, ConvexElems, {};
    )

    friend FArchive& operator<<(FArchive& Ar, FKAggregateGeom& V);


};

inline FArchive& operator<<(FArchive& Ar, FKAggregateGeom& V)
{
    {
        return Ar << V.SphereElems<< V.BoxElems << V.SphylElems << V.ConvexElems;
    }
}
