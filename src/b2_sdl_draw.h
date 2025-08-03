#pragma once

#include <box2d/box2d.h>
#include <SDL3/SDL.h>

struct b2AABB;

class b2SDLDraw : public b2Draw
{
public:
    b2SDLDraw(SDL_Renderer* renderer);

    void SetScale(float scale) { m_scale = scale; }
    float GetScale() const { return m_scale; }

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
    void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
    void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
    void DrawTransform(const b2Transform& xf) override;
    void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;
    void DrawString(int x, int y, const char* string, ...) override;
    void DrawAABB(b2AABB* aabb, const b2Color& color) override;

private:
    SDL_Renderer* m_renderer;
    float m_scale;
};
