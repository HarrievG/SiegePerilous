#pragma once

#include "box2d/id.h"
#include "tiled_data.h"

namespace SiegePerilous
{
    class PhysicsShapeCreator
    {
    public:
        virtual ~PhysicsShapeCreator() = default;
        virtual void create( b2WorldId worldId,
							const Tiled::Object& object,
							double offsetx = 0,
							double offsety = 0
							) const = 0;
    };

	// PhysicsShapeCreator for ASE sprites
	class AsepriteShapeCreator : public PhysicsShapeCreator {
	public:
		void create( b2WorldId worldId,
			const Tiled::Object &object,
			double offsetx = 0,
			double offsety = 0 ) const override;
	};
}


