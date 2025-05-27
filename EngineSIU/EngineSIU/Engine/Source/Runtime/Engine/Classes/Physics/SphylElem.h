#pragma once

#include "ShapeElem.h"

struct FKSphylElem : public FKShapeElem
{
    DECLARE_STRUCT(FKSphylElem, FKShapeElem)
    
    void Serialize(FArchive& Ar) override
    {
        FKShapeElem::Serialize(Ar);
    }
    friend FArchive& operator<<(FArchive& Ar, FKSphylElem& Elem);
};

inline FArchive& operator<<(FArchive& Ar, FKSphylElem& Elem)
{
    Ar.Serialize(Elem);
    return Ar;
}
