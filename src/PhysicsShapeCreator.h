#pragma once

#include "box2d/id.h"
#include "tiled_data.h"

namespace SiegePerilous
{
    class PhysicsShapeCreator
    {
    public:
        virtual ~PhysicsShapeCreator() = default;
        virtual void create(b2WorldId worldId, const Tiled::Object& object) const = 0;
    };
}
