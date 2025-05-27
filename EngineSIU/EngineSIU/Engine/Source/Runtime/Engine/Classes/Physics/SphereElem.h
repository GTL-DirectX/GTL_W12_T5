#pragma once

#include "ShapeElem.h"

struct FKSphereElem : public FKShapeElem
{
    DECLARE_STRUCT(FKSphereElem, FKShapeElem)

    void Serialize(FArchive& Ar) override
    {
        FKShapeElem::Serialize(Ar);
    }
    friend FArchive& operator<<(FArchive& Ar, FKSphereElem& Elem);
};

inline FArchive& operator<<(FArchive& Ar, FKSphereElem& Elem)
{
    Ar.Serialize(Elem);
    return Ar;
}
