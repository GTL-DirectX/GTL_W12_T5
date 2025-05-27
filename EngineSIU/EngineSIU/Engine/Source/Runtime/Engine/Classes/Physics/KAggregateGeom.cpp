#include "ShapeElem.h"
#include "BoxElem.h"
#include "ConvexElem.h"
#include "SphereElem.h"
#include "SphylElem.h"

void FKShapeElem::CloneElem(const FKShapeElem& Other)
{
    RestOffset = Other.RestOffset;
    ShapeType = Other.ShapeType;
    Name = Other.Name;
    bContributeToMass = Other.bContributeToMass;
    CollisionEnabled = Other.CollisionEnabled;
    bIsGenerated = Other.bIsGenerated;
}

FKConvexElem::FKConvexElem()
    : FKShapeElem(EAggCollisionShape::Convex)
{
}

FKConvexElem::FKConvexElem(const FKConvexElem& Other)
{
    CloneElem(Other);
}

FKConvexElem& FKConvexElem::operator=(const FKConvexElem& Other)
{
    CloneElem(Other);
    return *this;
}

void FKConvexElem::CloneElem(const FKConvexElem& Other)
{
    Super::CloneElem(Other);
    VertexData = Other.VertexData;
    IndexData = Other.IndexData;
    Box = Other.Box;
    Transform = Other.Transform;
}
