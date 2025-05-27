#pragma once
#include "BoxElem.h"
#include "ConvexElem.h"
#include "SphereElem.h"
#include "SphylElem.h"

struct FKAggregateGeom
{
    TArray<FKSphereElem> SphereElems;
    TArray<FKBoxElem> BoxElems;
    TArray<FKSphylElem> SphylElems;
    TArray<FKConvexElem> ConvexElems;

    friend FArchive& operator<<(FArchive& Ar, FKAggregateGeom& V);


};

inline FArchive& operator<<(FArchive& Ar, FKAggregateGeom& V)
{
    {
        return Ar << V.SphereElems<< V.BoxElems << V.SphylElems << V.ConvexElems;
    }
}
