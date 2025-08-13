#pragma once

#include "PhysicsShapeCreator.h"

namespace SiegePerilous
{
    class ChainShapeCreator : public PhysicsShapeCreator
    {
    public:
		void create( b2WorldId worldId,
			const Tiled::Object &object,
			double offsetx = 0,
			double offsety = 0) const override;
    };
}
