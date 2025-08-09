#pragma once

#include "PhysicsShapeCreator.h"

namespace SiegePerilous
{
    class ChainShapeCreator : public PhysicsShapeCreator
    {
    public:
        void create(b2WorldId worldId, const Tiled::Object& object) const override;
    };
}
