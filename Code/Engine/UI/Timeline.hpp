#pragma once
#include "Engine/UI/Widget.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include <vector>
#include <functional>

class Renderer;
class VBO;
class VertexBuffer;
class BitmapFont;
class Clock;

class Timeline : public Widget {
	friend class Overlay;
public:
	Timeline(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Clock& clockToUse, float timeSpan, const Rgba8& backgroundTint, const std::string& name, unsigned int animFPS = 0);
	virtual void RegisterIndicatorMoveCallbackFunction(std::function<void(float)> function);
	void SetClockToUseAndTimeSpanAndMaybeFPS(Clock& newClockToUse, float newTimeSpan, unsigned int animFPS = 0);

protected:
	virtual void Update() override;
	virtual void Render() const override;
	virtual ~Timeline();
	virtual void UpdateFromKeyboard();

protected:
	const Clock* m_clockToUse = nullptr;
	std::vector<Vertex_PCU> m_textVertices;
	VertexBuffer* m_textVBO = nullptr;
	BitmapFont* m_bitmapFont = nullptr;

	std::vector<Vertex_PCU> m_indicatorConvexPolyVerts;
	VertexBuffer* m_indicatorConvexPolyVBO = nullptr;
	ConvexPoly2D m_indicatorConvexPoly;

	std::vector<Vertex_PCU> m_indicatorMovingLineVerts;
	VertexBuffer* m_indicatorMovingLineVBO = nullptr;

	std::vector<Vertex_PCU> m_lineMarksVerts;
	VertexBuffer* m_lineMarksVBO = nullptr;

	float m_timeSpan = 0.0f;

	std::function<void(float)> m_registeredFunction = nullptr;

	bool m_isIndicatorBeingDragged = false;
	Vec2 m_offsetFromMouseClickedToPolyCenter;

	unsigned int m_animFPS = 0;
};