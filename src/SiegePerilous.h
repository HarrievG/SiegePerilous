#pragma once

#include "box2d/id.h"
#include "box2d/types.h"
#include "box2d/box2d.h"
#include "box2d/math_functions.h"


namespace SiegePerilous {

	struct physics_state {
		b2WorldId worldId;
		b2BodyId groundId;
		b2BodyId bodyId;
	};

	class WorldState {
	public:
		WorldState( ) : m_isInitialized( false ), m_isRunning( false ) { 		
		
		};
		~WorldState( ) { };

		bool Initialise( ) {
			if ( m_isInitialized ) {
				return true; // Already initialized
			}

			b2WorldDef worldDef = b2DefaultWorldDef( );
			worldDef.gravity = b2Vec2( { 0.0f, -9.8723f } );
			physicsState.worldId = b2CreateWorld( &worldDef );
			//
			//b2BodyDef groundBodyDef = b2DefaultBodyDef( );
			//groundBodyDef.position = b2Vec2( { 0.0f, -10.0f } );
			//
			//groundId = b2CreateBody( worldId, &groundBodyDef );
			//b2Polygon groundBox = b2MakeBox( 50.0f, 10.0f );
			//
			//b2ShapeDef groundShapeDef = b2DefaultShapeDef( );
			//b2CreatePolygonShape( groundId, &groundShapeDef, &groundBox );
			//
			//b2BodyDef bodyDef = b2DefaultBodyDef( );
			//bodyDef.type = b2_dynamicBody;
			//bodyDef.position = ( b2Vec2 ){ 0.0f, 4.0f };
			//bodyId = b2CreateBody( worldId, &bodyDef );
			//
			//b2ShapeDef shapeDef = b2DefaultShapeDef( );
			//shapeDef.density = 1.0f;
			//shapeDef.material.friction = 0.3f;
			//m_isInitialized = true;
			//
			//currentTime = hires_time_in_seconds( );
			//accumulator = 0.0;

			return true;
		}

		//void Shutdown( ) {
		//	if ( m_isInitialized ) {
		//		b2DestroyWorld( worldId );
		//		m_isInitialized = false;
		//	}
		//}

		//void Update( double deltaTime ) {
		//	if ( m_isRunning && m_isInitialized ) {
		//		b2World_Step(worldId, deltaTime, 4);
		//	}
		//}

		//bool IsRunning( ) const { return m_isRunning; }
		//void Start( ) { m_isRunning = true; }
		//void Stop( ) {
		//	m_isRunning = false;
	private:
		bool m_isInitialized;
		bool m_isRunning;

		physics_state physicsState;

		//double t = 0.0;
		//double dt = 0.01;

		//double currentTime;
		//double accumulator;

		//State previous;
		//State current;

	};
}
