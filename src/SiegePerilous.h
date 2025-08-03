#pragma once

#include "box2d/id.h"
#include "box2d/types.h"
#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include <SDL3/SDL.h>


namespace SiegePerilous {

	struct PhysicsState {
		b2WorldId worldId;
		b2BodyId groundId;
		b2BodyId bodyId;
	};

	struct AudioState {
		SDL_AudioStream *stream_in;
		SDL_AudioStream *stream_out;
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

			b2BodyDef groundBodyDef = b2DefaultBodyDef( );
			groundBodyDef.position = b2Vec2( { 0.0f, -10.0f } );

			physicsState.groundId = b2CreateBody( physicsState.worldId, &groundBodyDef );
			b2Polygon groundBox = b2MakeBox( 50.0f, 10.0f );

			b2ShapeDef groundShapeDef = b2DefaultShapeDef( );
			b2CreatePolygonShape( physicsState.groundId, &groundShapeDef, &groundBox );

			b2BodyDef bodyDef = b2DefaultBodyDef( );
			bodyDef.type = b2_dynamicBody;
			bodyDef.position = b2Vec2({ 0.0f, 4.0f });
			physicsState.bodyId = b2CreateBody( physicsState.worldId, &bodyDef );

			b2ShapeDef shapeDef = b2DefaultShapeDef( );
			shapeDef.density = 1.0f;
			shapeDef.material.friction = 0.3f;

			m_isInitialized = true;

			currentTime =  SDL_GetTicks() / 1000.0;
			accumulator = 0.0;

			return true;
		}

		void Shutdown( ) {
			if ( m_isInitialized ) {
				b2DestroyWorld( physicsState.worldId );
				m_isInitialized = false;
			}
		}

		void Update( ) {
			double newTime = SDL_GetTicks() / 1000.0;
			double frameTime = newTime - currentTime;
			currentTime = newTime;
			accumulator += frameTime;

			const double timeStep = 1.0 / 60.0;
			while (accumulator >= timeStep)
			{
				b2World_Step(physicsState.worldId, timeStep, 4);
				accumulator -= timeStep;
			}
		}

		bool IsRunning( ) const { return m_isRunning; }
		void Start( ) { m_isRunning = true; }
		void Stop( ) { m_isRunning = false; }

		PhysicsState physicsState;
		AudioState audioState;

	private:
		bool m_isInitialized;
		bool m_isRunning;

		double currentTime;
		double accumulator;
	};
}
