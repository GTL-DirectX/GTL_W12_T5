#pragma once

#include "ShapeElem.h"

struct FKConvexElem : public FKShapeElem
{
    DECLARE_STRUCT(FKConvexElem, FKShapeElem)

    TArray<FVector> VertexData; // Convex shape vertices in local space

    TArray<int32> IndexData;

    FBoundingBox Box;

private:
    FTransform Transform = FTransform::Identity; // Transform to apply to the convex shape


public:

    FKConvexElem();
    FKConvexElem(const FKConvexElem& Other);
    FKConvexElem& operator=(const FKConvexElem& Other);


private:
    void CloneElem(const FKConvexElem& Other);
    
};
