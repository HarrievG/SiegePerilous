#pragma once

#include "box2d/id.h"
#include "box2d/types.h"
#include "box2d/box2d.h"
#include <SDL3/SDL.h>

struct b2AABB;

// --- Constants ---
const float PIXELS_PER_METER = 20.0f;
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

// Converts radians to degrees
constexpr float radToDeg( float radians ) {
	return radians * 180.0f / SDL_PI_F;
}

// Converts degrees to radians
constexpr float degToRad( float degrees ) {
	return degrees * SDL_PI_F / -180.0f;
}

struct PhysicsState {
	b2WorldId worldId;
	b2BodyId groundId;
	b2BodyId bodyId;
};


#include <box2d/box2d.h>
#include <SDL3/SDL.h>

class Camera {
public:
	// Constructor uses the renderer to query its output size.
	// if no renderer is given, width and height must be provided.
	Camera( SDL_Renderer *renderer, int width = 1920,int height= 1080 );

	// Resets camera to default position and zoom.
	void ResetView( );

	void UpdateRenderer(SDL_Renderer * renderer);

	// Converts a point from screen coordinates (pixels, Y-down) to world coordinates (meters, Y-up).
	b2Vec2 ConvertScreenToWorld( const SDL_FPoint &screenPoint ) const;

	// Converts a point from world coordinates (meters, Y-up) to screen coordinates (pixels, Y-down).
	SDL_FPoint ConvertWorldToScreen( const b2Vec2 &worldPoint ) const;

	// Calculates the visible area of the world.
	b2AABB GetViewBounds( ) const;

	// A helper to get the current scale factor. Useful for sizing objects.
	float GetPixelsPerMeter( ) const;

	// --- Camera Controls ---
	void SetZoom( float zoom ) { m_zoom = b2MaxFloat( 0.01f, zoom ); }
	void SetCenter( const b2Vec2 &center ) { m_center = center; }
	float GetZoom( ) const { return m_zoom; }
	const b2Vec2 &GetCenter( ) const { return m_center; }
	SDL_Renderer *GetRenderer( ) const { return m_renderer; }
private:
	SDL_Renderer *m_renderer;
	int m_width;
	int m_height;

	b2Vec2 m_center; // Camera's center position in world coordinates (meters)
	float m_zoom;    // Controls the view size. Smaller value = more zoomed in.
};

class b2SDLDraw
{
public:
    b2SDLDraw(SDL_Renderer* renderer,Camera * camera);

    void SetScale(float scale) { m_scale = scale; }
    float GetScale() const { return m_scale; }
	void SetFlags( uint32_t flags ) { /*m_debugDraw.SetFlags( flags );*/ }
	void DrawSegment( b2Vec2 p1, b2Vec2 p2, b2HexColor color, void *ctx );
	void DrawTransform( b2Transform xf, void *ctx );
	void DrawPolygon( const b2Vec2 *vertices, int count, b2HexColor color, void *ctx );
	void DrawSolidPolygon( b2Transform transform, const b2Vec2 *vertices, int vertexCount, float radius, b2HexColor color, void *ctx );
	void DrawCircle( b2Vec2 center, float radius, b2HexColor color, void *ctx );
	void DrawSolidCircle( b2Transform transform, b2Vec2 center, float radius, b2HexColor color,void *ctx );
	void DrawPoint( b2Vec2 p, float size, b2HexColor color, void *context );

	b2DebugDraw * getDebugDrawPtr( ) { return &m_debugDraw; }
	SDL_Renderer * getDrawPtr() { return m_renderer; }
private:
	Camera *m_camera;
    SDL_Renderer* m_renderer;
	b2DebugDraw m_debugDraw;
    float m_scale;
};
