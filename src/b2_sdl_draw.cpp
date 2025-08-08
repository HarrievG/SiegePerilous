#include "b2_sdl_draw.h"

#include "box2d/id.h"
#include "box2d/types.h"
#include "box2d/box2d.h"
#include "box2d/base.h"

#include <vector>
#include <cstdarg>


static void SDLColorFromHex( SDL_Renderer *renderer, b2HexColor hex ) {
	Uint8 r = ( hex >> 16 ) & 0xFF;
	Uint8 g = ( hex >> 8 ) & 0xFF;
	Uint8 b = ( hex ) & 0xFF;
	SDL_SetRenderDrawColor( renderer, r, g, b, 255 );
}

void b2SDLDraw::DrawSegment( b2Vec2 p1, b2Vec2 p2, b2HexColor color, void *ctx ) {
	SDL_Renderer *renderer = ( SDL_Renderer * ) ctx;
	SDLColorFromHex( renderer, color );


	auto sp1 = m_camera->ConvertWorldToScreen( p1 );
	auto sp2 = m_camera->ConvertWorldToScreen(p2 );

	SDL_RenderLine(renderer,sp1.x,sp1.y,sp2.x,sp2.y);
}

void b2SDLDraw::DrawTransform( b2Transform xf, void *ctx ) {
	SDL_Renderer *renderer = ( SDL_Renderer * ) ctx;

	auto sp1 = m_camera->ConvertWorldToScreen( xf.p );
	auto sp2x = m_camera->ConvertWorldToScreen( { xf.p.x + xf.q.c, xf.p.y + xf.q.s } );
	auto sp2y = m_camera->ConvertWorldToScreen( { xf.p.x - xf.q.s, xf.p.y + xf.q.c } );

	SDLColorFromHex( renderer, b2HexColor::b2_colorBox2DRed ); // x axis - red
	SDL_RenderLine( renderer, sp1.x,sp1.y, sp2x.x , sp2x.y);

	SDLColorFromHex( renderer, b2HexColor::b2_colorBox2DGreen ); // y axis - green
	SDL_RenderLine( renderer, sp1.x,sp1.y, sp2y.x , sp2y.y);
}

void b2SDLDraw::DrawPolygon( const b2Vec2 *vertices, int count, b2HexColor color, void *ctx ) {
	SDL_Renderer *renderer = ( SDL_Renderer * ) ctx;
	SDLColorFromHex( renderer, color );

	int width, height;
	SDL_GetCurrentRenderOutputSize( m_renderer, &width, &height );

	for ( int i = 0; i < count; ++i ) {
		int j = ( i + 1 ) % count;
		auto sp1  = m_camera->ConvertWorldToScreen( vertices[i] );
		auto sp2 = m_camera->ConvertWorldToScreen( vertices[j] );
		SDL_RenderLine( renderer, sp1.x, sp1.y, sp2.x, sp2.y );
	}
}

void b2SDLDraw::DrawSolidPolygon( b2Transform transform, const b2Vec2 *vertices, int vertexCount, float radius, b2HexColor color, void *ctx ) {

}

void b2SDLDraw::DrawCircle( b2Vec2 center, float radius, b2HexColor color, void *ctx ) {
	SDL_Renderer *renderer = ( SDL_Renderer * ) ctx;
	SDLColorFromHex( renderer, color );
	int width, height;
	SDL_GetCurrentRenderOutputSize( m_renderer, &width, &height );

	const int segments = 32;
	float angleStep = 2.0f * 3.14159f / segments;
	for ( int i = 0; i < segments; ++i ) {
		float angle1 = i * angleStep;
		float angle2 = ( i + 1 ) * angleStep;
		b2Vec2 p1 = { center.x + radius * cosf( angle1 ), center.y + radius * sinf( angle1 ) };
		b2Vec2 p2 = { center.x + radius * cosf( angle2 ), center.y + radius * sinf( angle2 ) };

		auto sp1 = m_camera->ConvertWorldToScreen( p1 );
		auto sp2 = m_camera->ConvertWorldToScreen( p2 );

		SDL_RenderLine( renderer,
			sp1.x, sp1.y,
			sp2.x, sp2.y );
	}
}

void b2SDLDraw::DrawSolidCircle( b2Transform transform, b2Vec2 center, float radius, b2HexColor color, void *ctx ) {

	transform.p = b2TransformPoint( transform, center );
	DrawCircle(transform.p,radius,color,ctx);
}

void b2SDLDraw::DrawPoint( b2Vec2 p, float size, b2HexColor color, void *ctx ) {
		SDL_Renderer* renderer = static_cast<SDL_Renderer*>(ctx);
		SDLColorFromHex(renderer, color);

		// 1. Convert the center of the point from world to screen coordinates
		SDL_FPoint screenCenter = m_camera->ConvertWorldToScreen(p);

		// 2. Convert the world size (meters) to a screen size (pixels)
		// The 'size' from b2Draw is typically a small diameter.
		float pixelSize = size;

		// Make sure the point is at least 1 pixel big so it's always visible
		if (pixelSize < 1.0f) {
			pixelSize = 1.0f;
		}

		// 3. Create an SDL_FRect centered on the point
		SDL_FRect pointRect;
		pointRect.w = pixelSize;
		pointRect.h = pixelSize;
		pointRect.x = screenCenter.x - (pixelSize / 2.0f);
		pointRect.y = screenCenter.y - (pixelSize / 2.0f);

		// 4. Draw the filled rectangle
		SDL_RenderFillRect(renderer, &pointRect);
}

b2SDLDraw::b2SDLDraw(SDL_Renderer* renderer,Camera * camera)
    : m_camera(camera), m_renderer( renderer ), m_scale( 20.0f )
{
	m_debugDraw = b2DefaultDebugDraw( );
	m_debugDraw.context = this;
	
	m_debugDraw.DrawSegmentFcn = [] ( b2Vec2 p1, b2Vec2 p2, b2HexColor color, void *ctx ) {
		b2SDLDraw *self = static_cast< b2SDLDraw * >( ctx );
		self->DrawSegment( p1, p2, color, (void*)self->getDrawPtr() );
		};

	m_debugDraw.DrawCircleFcn = [] ( b2Vec2 center, float radius, b2HexColor color, void *ctx ) {
			b2SDLDraw *self = static_cast< b2SDLDraw * >( ctx );
			self->DrawCircle( center, radius, color, ( void * ) self->getDrawPtr( ) );
		};

	m_debugDraw.DrawSolidCircleFcn = [] ( b2Transform transform, float radius, b2HexColor color, void* ctx ) {
		b2SDLDraw *self = static_cast< b2SDLDraw * >( ctx );
		self->DrawSolidCircle(transform,b2Vec2_zero, radius, color, ( void * ) self->getDrawPtr( ) );
		};

	m_debugDraw.DrawPolygonFcn = [] ( const b2Vec2 *vertices, int count, b2HexColor color, void *ctx ) {
		b2SDLDraw *self = static_cast< b2SDLDraw * >( ctx );
		self->DrawPolygon( vertices, count, color, ( void * ) self->getDrawPtr( ) );
		};
	m_debugDraw.DrawSolidPolygonFcn = [] ( b2Transform transform, const b2Vec2 *vertices, int count, float radius, b2HexColor color, void *ctx ) {
		b2SDLDraw *self = static_cast< b2SDLDraw * >( ctx );
			self->DrawPolygon(vertices, count, color, ( void * ) self->getDrawPtr( ) );
		}; 

	m_debugDraw.DrawTransformFcn = [] ( b2Transform xf, void *ctx  ) {
		b2SDLDraw *self = static_cast< b2SDLDraw * >( ctx );
		self->DrawTransform(xf, (void*)self->getDrawPtr()  );
		};

	m_debugDraw.DrawPointFcn = [] ( b2Vec2 p, float size, b2HexColor color, void *ctx ) {
		b2SDLDraw *self = static_cast< b2SDLDraw * >( ctx );
		self->DrawPoint( p, size, color, ( void * ) self->getDrawPtr( ) );
		};

	/// Option to draw shapes
	m_debugDraw.drawShapes= true;
	/// Option to draw joints
	m_debugDraw.drawJoints= true;
	/// Option to draw additional information for joints
	m_debugDraw.drawJointExtras= true;
	/// Option to draw the bounding boxes for shapes
	m_debugDraw.drawBounds= true;
	/// Option to draw the mass and center of mass of dynamic bodies
	m_debugDraw.drawMass= true;
	/// Option to draw body names
	m_debugDraw.drawBodyNames= true;
	/// Option to draw contact points
	m_debugDraw.drawContacts= true;
	/// Option to visualize the graph coloring used for contacts and joints
	m_debugDraw.drawGraphColors= true;
	/// Option to draw contact normals
	m_debugDraw.drawContactNormals= true;
	/// Option to draw contact normal impulses
	m_debugDraw.drawContactImpulses= true;
	/// Option to draw contact feature ids
	m_debugDraw.drawContactFeatures= true;
	/// Option to draw contact friction impulses
	m_debugDraw.drawFrictionImpulses= true;
	/// Option to draw islands as bounding boxes
	m_debugDraw.drawIslands = true;
}

float Camera::GetPixelsPerMeter( ) const {
	// The total visible height in meters is (m_zoom * 2).
	// The total visible height in pixels is m_height.
	// So, pixels per meter is m_height / (2 * m_zoom).
	return static_cast< float >( m_height ) / ( m_zoom * 2.0f );
}

void Camera::ResetView( ) {
	m_center = { 0.0f, 0.0f };
	// The original 'zoom' of 20 seems to represent the half-height of the view in meters.
	// So the total visible height is 40 meters.
	m_zoom = 20.0f;
}

void Camera::UpdateRenderer( SDL_Renderer *renderer ) {
	if ( m_renderer != renderer ) {
		m_renderer = renderer;
		SDL_GetRenderOutputSize( m_renderer, &m_width, &m_height );
		if ( m_width == 0 || m_height == 0 ) {
			SDL_LogWarn( SDL_LOG_CATEGORY_APPLICATION, "Couldn't SDL_GetRenderOutputSize %s", SDL_GetError( ) );
		}
	}
}

b2Vec2 Camera::ConvertScreenToWorld( const SDL_FPoint &screenPoint ) const {
	float ppm = GetPixelsPerMeter( );
	if ( ppm == 0.0f ) return { 0.0f, 0.0f }; // Avoid division by zero

	// 1. Find the point's position relative to the center of the screen (in pixels)
	float pixelX = screenPoint.x - ( m_width / 2.0f );
	float pixelY = screenPoint.y - ( m_height / 2.0f );

	// 2. Scale from pixels to meters and flip the Y-axis
	float meterX = pixelX / ppm;
	float meterY = -pixelY / ppm; // Negate to convert from SDL's Y-down to Box2D's Y-up

	// 3. Add the camera's world offset to get the final world coordinate
	return { m_center.x + meterX, m_center.y + meterY };
}

b2AABB Camera::GetViewBounds( ) const {
	// The bounds are simply the world coordinates of the screen's corners.
	b2AABB bounds;
	bounds.lowerBound = ConvertScreenToWorld( { 0.0f, static_cast< float >( m_height ) } ); // Bottom-left
	bounds.upperBound = ConvertScreenToWorld( { static_cast< float >( m_width ), 0.0f } );   // Top-right
	return bounds;
}

SDL_FPoint Camera::ConvertWorldToScreen( const b2Vec2 &worldPoint ) const {
	float ppm = GetPixelsPerMeter( );

	// 1. Find the point's position relative to the camera's center (in meters)
	b2Vec2 relativePos_meters = worldPoint - m_center;

	// 2. Scale from meters to pixels
	float pixelX = relativePos_meters.x * ppm;
	float pixelY = relativePos_meters.y * ppm;

	// 3. Translate to screen space.
	// The center of the screen is our new origin.
	// We subtract pixelY because SDL's Y-axis is inverted (points down).
	float screenX = ( m_width / 2.0f ) + pixelX;
	float screenY = ( m_height / 2.0f ) - pixelY;

	return { screenX, screenY };
}

Camera::Camera( SDL_Renderer *renderer , int width/* = 1920*/,int height/*= 1080 */) : m_renderer( renderer ) {
	// Get the logical size of the renderer's output.
	// This correctly handles high-DPI displays and different render scales.
	SDL_GetRenderOutputSize( m_renderer, &m_width, &m_height );
	if ( m_width == 0 || m_height == 0 ) {
		SDL_LogWarn( SDL_LOG_CATEGORY_APPLICATION, "Couldn't SDL_GetRenderOutputSize %s", SDL_GetError( ) );
		// Fallback for safety, though this shouldn't happen with a valid renderer.
		m_width = width;
		m_height = height;
	}
	ResetView( );
}
