#pragma once

#include "box2d/id.h"
#include "box2d/types.h"
#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include <SDL3/SDL.h>
#include "b2_sdl_draw.h"


namespace SiegePerilous {

	struct AudioState {
		SDL_AudioStream *stream_in;
		SDL_AudioStream *stream_out;
	};

	class WorldState {
	public:
		WorldState( SDL_Renderer *renderer );;
		~WorldState( );;

		void SetRenderer( SDL_Renderer *renderer );

		bool Initialise( );

		void Shutdown( );

		void Update( );

		void Draw( );

		bool IsRunning( ) const { return m_isRunning; }
		void Start( );
		void Stop( );

		PhysicsState physicsState;
		AudioState audioState;

		Camera &GetCamera( ) { return m_camera; }
	private:
		bool m_isInitialized;
		bool m_isRunning;
		b2SDLDraw *m_debugDraw;

		double currentTime;
		double accumulator;

		Camera m_camera;
	};
}
