#include "ChainShapeCreator.h"
#include "box2d/id.h"
#include "box2d/types.h"
#include "box2d/box2d.h"
#include "box2d/math_functions.h"

#include <vector>

namespace SiegePerilous
{
	inline b2Vec2 b2Mul( const b2Rot &q, const b2Vec2 &v ) {
		return b2Vec2( q.c * v.x - q.s * v.y, q.s * v.x + q.c * v.y );
	}

	void ChainShapeCreator::create(b2WorldId worldId, const Tiled::Object &object, double offsetx /*= 0*/, double offsety /*= 0*/ ) const {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

		auto rotation = b2MakeRot(object.rotation * ( 3.14159265358979323846f / 180.0f )); // Convert degrees to radians
        std::vector<b2Vec2> points;
        if (object.polygon)
        {
			for (const auto& point : *object.polygon)
			{
				b2Vec2 objVec = { (float)object.x + offsetx , (float) object.y + offsety };
				b2Vec2 objPoint =b2Mul( rotation, b2Vec2(point.x,point.y )) + objVec  ;
				
				// 3. convert  screen coordinates to world coordinates ( refactor this to use camera )
				//////////////////////////////////////////////////////////////////////////				
				float ppm = ( 1080 ) / ( 20 * 2.0f ); 
				// 3.1. Find the point's position relative to the center of the screen (in pixels)
				float pixelX = objPoint.x - ( 1920. / 2.0f );
				float pixelY = objPoint.y - ( 1080. / 2.0f );
				// 3.2. Scale from pixels to meters and flip the Y-axis
				float meterX = pixelX / ppm;
				float meterY = -pixelY / ppm; // Negate to convert from SDL's Y-down to Box2D's Y-up
				//////////////////////////////////////////////////////////////////////////

				points.push_back( b2Vec2(meterX,meterY) );
			}
        }
		b2ChainDef chainDef = b2DefaultChainDef();
		chainDef.points = points.data( );
		chainDef.count = static_cast< int >( points.size( ) );	
		chainDef.isLoop = true;

		b2CreateChain(bodyId, &chainDef);
    }


	void AsepriteShapeCreator::create( b2WorldId worldId, const Tiled::Object &object, double offsetx /*= 0*/, double offsety /*= 0*/ ) const {
		b2BodyDef bodyDef = b2DefaultBodyDef( );
		b2BodyId bodyId = b2CreateBody( worldId, &bodyDef );

		auto rotation = b2MakeRot( object.rotation * ( 3.14159265358979323846f / 180.0f ) ); // Convert degrees to radians
		std::vector<b2Vec2> points;
		if ( object.polygon ) {
			for ( const auto &point : *object.polygon ) {
				b2Vec2 objVec = { ( float ) object.x + offsetx , ( float ) object.y + offsety };
				b2Vec2 objPoint = b2Mul( rotation, b2Vec2( point.x, point.y ) ) + objVec;

				// 3. convert  screen coordinates to world coordinates ( refactor this to use camera )
				//////////////////////////////////////////////////////////////////////////				
				float ppm = ( 1080 ) / ( 20 * 2.0f );
				// 3.1. Find the point's position relative to the center of the screen (in pixels)
				float pixelX = objPoint.x - ( 1920. / 2.0f );
				float pixelY = objPoint.y - ( 1080. / 2.0f );
				// 3.2. Scale from pixels to meters and flip the Y-axis
				float meterX = pixelX / ppm;
				float meterY = -pixelY / ppm; // Negate to convert from SDL's Y-down to Box2D's Y-up
				//////////////////////////////////////////////////////////////////////////

				points.push_back( b2Vec2( meterX, meterY ) );
			}
		}
		b2ChainDef chainDef = b2DefaultChainDef( );
		chainDef.points = points.data( );
		chainDef.count = static_cast< int >( points.size( ) );
		chainDef.isLoop = true;

		b2CreateChain( bodyId, &chainDef );
	}

}
