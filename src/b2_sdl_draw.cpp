#include "b2_sdl_draw.h"
#include <vector>
#include <cstdarg>

b2SDLDraw::b2SDLDraw(SDL_Renderer* renderer)
    : m_renderer(renderer), m_scale(20.0f)
{
}

void b2SDLDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    int width, height;
    SDL_GetRendererOutputSize(m_renderer, &width, &height);

    std::vector<SDL_FPoint> points(vertexCount + 1);
    for (int32 i = 0; i < vertexCount; ++i)
    {
        points[i].x = (width / 2.0f) + vertices[i].x * m_scale;
        points[i].y = (height / 2.0f) - vertices[i].y * m_scale;
    }
    points[vertexCount] = points[0]; // Close the loop

    SDL_SetRenderDrawColor(m_renderer, (Uint8)(color.r * 255), (Uint8)(color.g * 255), (Uint8)(color.b * 255), (Uint8)(color.a * 255));
    SDL_RenderLines(m_renderer, points.data(), vertexCount + 1);
}

void b2SDLDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    // NOTE: This does not draw a filled polygon yet. It draws a semi-transparent outline.
    // For a real fill, SDL_RenderGeometry would be needed.

    // Draw a semi-transparent background
    b2Color fillColor = color;
    fillColor.a *= 0.5f;
    DrawPolygon(vertices, vertexCount, fillColor);

    // Draw the solid outline
    DrawPolygon(vertices, vertexCount, color);
}

void b2SDLDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
    int width, height;
    SDL_GetRendererOutputSize(m_renderer, &width, &height);

    const int32 k_segments = 32;
    std::vector<SDL_FPoint> points(k_segments + 1);
    const float k_increment = 2.0f * b2_pi / k_segments;
    float theta = 0.0f;
    for (int32 i = 0; i < k_segments; ++i)
    {
        b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
        points[i].x = (width / 2.0f) + v.x * m_scale;
        points[i].y = (height / 2.0f) - v.y * m_scale;
        theta += k_increment;
    }
    points[k_segments] = points[0];

    SDL_SetRenderDrawColor(m_renderer, (Uint8)(color.r * 255), (Uint8)(color.g * 255), (Uint8)(color.b * 255), (Uint8)(color.a * 255));
    SDL_RenderLines(m_renderer, points.data(), k_segments + 1);
}

void b2SDLDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
    // NOTE: This does not draw a filled circle yet. It draws a semi-transparent outline.
    b2Color fillColor = color;
    fillColor.a *= 0.5f;
    DrawCircle(center, radius, fillColor);

    // Draw the solid outline
    DrawCircle(center, radius, color);

    // Draw the axis line
    b2Vec2 p = center + radius * axis;
    DrawSegment(center, p, color);
}

void b2SDLDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    int width, height;
    SDL_GetRendererOutputSize(m_renderer, &width, &height);

    float x1 = (width / 2.0f) + p1.x * m_scale;
    float y1 = (height / 2.0f) - p1.y * m_scale;
    float x2 = (width / 2.0f) + p2.x * m_scale;
    float y2 = (height / 2.0f) - p2.y * m_scale;

    SDL_SetRenderDrawColor(m_renderer, (Uint8)(color.r * 255), (Uint8)(color.g * 255), (Uint8)(color.b * 255), (Uint8)(color.a * 255));
    SDL_RenderLine(m_renderer, x1, y1, x2, y2);
}

void b2SDLDraw::DrawTransform(const b2Transform& xf)
{
    const float k_axisScale = 0.4f;
    b2Vec2 p1 = xf.p;

    b2Vec2 p2 = p1 + k_axisScale * xf.q.GetXAxis();
    DrawSegment(p1, p2, b2Color(1.0f, 0.0f, 0.0f, 1.0f)); // Red for X-axis

    p2 = p1 + k_axisScale * xf.q.GetYAxis();
    DrawSegment(p1, p2, b2Color(0.0f, 1.0f, 0.0f, 1.0f)); // Green for Y-axis
}

void b2SDLDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
    int width, height;
    SDL_GetRendererOutputSize(m_renderer, &width, &height);

    float x = (width / 2.0f) + p.x * m_scale;
    float y = (height / 2.0f) - p.y * m_scale;

    SDL_SetRenderDrawColor(m_renderer, (Uint8)(color.r * 255), (Uint8)(color.g * 255), (Uint8)(color.b * 255), (Uint8)(color.a * 255));
    SDL_RenderPoint(m_renderer, x, y);
}

void b2SDLDraw::DrawString(int x, int y, const char* string, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, string);
    vsnprintf(buffer, sizeof(buffer), string, args);
    va_end(args);

    // We assume x and y are already in screen coordinates
    SDL_RenderDebugText(m_renderer, x, y, buffer);
}

void b2SDLDraw::DrawAABB(b2AABB* aabb, const b2Color& color)
{
    int width, height;
    SDL_GetRendererOutputSize(m_renderer, &width, &height);

    SDL_FRect rect;
    rect.x = (width / 2.0f) + aabb->lowerBound.x * m_scale;
    rect.y = (height / 2.0f) - aabb->upperBound.y * m_scale;
    rect.w = (aabb->upperBound.x - aabb->lowerBound.x) * m_scale;
    rect.h = (aabb->upperBound.y - aabb->lowerBound.y) * m_scale;

    SDL_SetRenderDrawColor(m_renderer, (Uint8)(color.r * 255), (Uint8)(color.g * 255), (Uint8)(color.b * 255), (Uint8)(color.a * 255));
    SDL_RenderRect(m_renderer, &rect);
}
