
#include "World.h"
#include "tiled_data.h"

SiegePerilous::WorldState::WorldState( SDL_Renderer *renderer ) : m_isInitialized( false ), m_isRunning( false ), m_debugDraw( nullptr ), m_camera( renderer ) {
	SetRenderer( renderer );
	physicsState.worldId = B2_NULL_ID;
	physicsState.groundId = B2_NULL_ID;
	physicsState.bodyId = B2_NULL_ID;
	audioState.stream_in = nullptr;
	audioState.stream_out = nullptr;
	currentTime = 0.0;
	accumulator = 0.0;
}

SiegePerilous::WorldState::~WorldState( ) {
	if ( m_isInitialized ) {
		Shutdown( );
	}

	if ( m_debugDraw ) {
		delete m_debugDraw;
		m_debugDraw = nullptr;
	}
}

void SiegePerilous::WorldState::SetRenderer( SDL_Renderer *renderer ) {
	m_debugDraw = new b2SDLDraw( renderer, &m_camera );
	uint32_t flags = 0;
	//flags |= b2_drawShapes;
	//flags |= b2_drawJoints;
	//flags |= b2_drawAABBs;
	//flags |= b2_drawMass;
	//flags |= b2_drawTransforms;
	m_debugDraw->SetFlags( flags );
}

bool SiegePerilous::WorldState::Initialise( ) {
	if ( m_isInitialized ) {
		return true; // Already initialized
	}

	b2WorldDef worldDef = b2DefaultWorldDef( );
	worldDef.gravity = b2Vec2( { 0.0f, -10.f } );
	physicsState.worldId = b2CreateWorld( &worldDef );

	b2BodyDef groundBodyDef = b2DefaultBodyDef( );
	groundBodyDef.position = b2Vec2( { 0.0f, 0.0f } );
	physicsState.groundId = b2CreateBody( physicsState.worldId, &groundBodyDef );
	b2Polygon groundBox = b2MakeOffsetBox( 25.0f, 1.0f, { 0.0f,0.0f }, b2MakeRot( degToRad( 10 ) ) );
	b2ShapeDef groundShapeDef = b2DefaultShapeDef( );
	b2CreatePolygonShape( physicsState.groundId, &groundShapeDef, &groundBox );

	b2BodyDef bodyDef = b2DefaultBodyDef( );
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Vec2( { 0.0f, 10.0f } );
	physicsState.bodyId = b2CreateBody( physicsState.worldId, &bodyDef );
	b2Circle circle = { {0.0f, 0.0f}, 0.5f };
	b2ShapeDef shapeDef = b2DefaultShapeDef( );
	shapeDef.density = 1.0f;
	shapeDef.material.friction = 0.3f;
	b2CreateCircleShape( physicsState.bodyId, &shapeDef, &circle );


	m_isInitialized = true;

	currentTime = SDL_GetTicks( ) / 1000.0;
	accumulator = 0.0;

	auto map = Tiled::load_map_with_deps("../../../content/main_menu.json" );

	return true;
}

void SiegePerilous::WorldState::Shutdown( ) {
	if ( m_isInitialized ) {
		b2DestroyWorld( physicsState.worldId );
		delete m_debugDraw;
		m_debugDraw = nullptr;
		m_isInitialized = false;
	}
}

void SiegePerilous::WorldState::Update( ) {
	double newTime = SDL_GetTicks( ) / 1000.0;
	double frameTime = newTime - currentTime;
	currentTime = newTime;
	accumulator += frameTime;

	const float timeStep = 1.0 / 60.0;
	while ( accumulator >= timeStep ) {
		b2World_Step( physicsState.worldId, timeStep, 4 );
		accumulator -= timeStep;
	}
}

void SiegePerilous::WorldState::Draw( ) {
	if ( m_debugDraw ) {
		b2World_Draw( physicsState.worldId, m_debugDraw->getDebugDrawPtr( ) );
	}
}

void SiegePerilous::WorldState::Start( ) {
	m_isRunning = true;
}

void SiegePerilous::WorldState::Stop( ) {
	m_isRunning = false;
}
