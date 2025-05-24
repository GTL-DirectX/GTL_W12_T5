#pragma once


struct FKAggregateGeom
{
    TArray<FKSphereElem> SphereElems;
    TArray<FKBoxElem> BoxElems;
    TArray<FKSphylElem> SphylElems;
    TArray<FKConvexElem> ConvexElems;
};
