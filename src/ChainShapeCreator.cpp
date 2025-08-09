#include "ChainShapeCreator.h"
#include "box2d/box2d.h"
#include <vector>

namespace SiegePerilous
{
    void ChainShapeCreator::create(b2WorldId worldId, const Tiled::Object& object) const
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

        std::vector<b2Vec2> points;
        if (object.polygon)
        {
            for (const auto& point : *object.polygon)
            {
                points.push_back({ (object.x + point.x) / 100.0f, -(object.y + point.y) / 100.0f });
            }
        }

        b2ChainShape chain = b2CreateChain(points.data(), points.size(), false);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreateChainShape(bodyId, &shapeDef, &chain);
    }
}
