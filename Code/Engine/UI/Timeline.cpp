#include "Engine/UI/Timeline.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"

Timeline::Timeline(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Clock& clockToUse, float timeSpan, const Rgba8& backgroundTint, const std::string& name, unsigned int animFPS)
	: Widget(rendererToUse, overlay, screenSpaceAABB2, backgroundTint, name), m_timeSpan(timeSpan), m_clockToUse(&clockToUse), m_animFPS(animFPS)
{
	m_widgetType = WidgetType::TIMELINE;
	GUARANTEE_OR_DIE(timeSpan > 0.0f, "timeSpan <= 0.0f doesn't make sense!");
	m_bitmapFont = rendererToUse.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	GUARANTEE_OR_DIE(m_bitmapFont != nullptr, "Data/Fonts/SquirrelFixedFont doesn't exist");

	std::vector<Vec2> convexPolyPoints;
	convexPolyPoints.push_back(Vec2(10.0f, 10.0f));
	convexPolyPoints.push_back(Vec2(-10.0f, 10.0f));
	convexPolyPoints.push_back(Vec2(-10.0f, -10.0f));
	convexPolyPoints.push_back(Vec2(0.0f, -15.0f));
	convexPolyPoints.push_back(Vec2(10.0f, -10.0f));
	m_indicatorConvexPoly.m_points = convexPolyPoints;

	AddVertsForConvexPoly2D(m_indicatorConvexPolyVerts, m_indicatorConvexPoly, Rgba8::MAROON);
	Vec2 centerPos = m_indicatorConvexPoly.GetCenterPos();
	//AddVertsForLineSegment2D(m_indicatorConvexPolyVerts, centerPos, Vec2(centerPos.x, 0.0f), 1.5f, Rgba8::GREY);
	m_indicatorConvexPolyVBO = m_renderer.CreateVertexBuffer(m_indicatorConvexPolyVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "IndicatorConvexPolyVBO");
	m_renderer.CopyCPUToGPU(m_indicatorConvexPolyVerts.data(), m_indicatorConvexPolyVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_indicatorConvexPolyVBO);
	
	m_indicatorMovingLineVerts.push_back(Vertex_PCU(centerPos, Rgba8::GREY));
	m_indicatorMovingLineVerts.push_back(Vertex_PCU(Vec2(centerPos.x, 0.0f), Rgba8::GREY));
	m_indicatorMovingLineVBO = m_renderer.CreateVertexBuffer(m_indicatorMovingLineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "IndicatorMovingLineVBO", false);
	m_renderer.CopyCPUToGPU(m_indicatorMovingLineVerts.data(), m_indicatorMovingLineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_indicatorMovingLineVBO);

	if (animFPS > 0) {
		float finalFrameNum = timeSpan * (float)animFPS;
		float inv_finalFrameNum = 1.0f / finalFrameNum;
		for (unsigned int i = 0; i <= (unsigned int)finalFrameNum; i += 5) {
			float uv_x = (float)i * inv_finalFrameNum;
			Vec2 markStart = m_screenSpaceAABB2.GetPointAtUV(Vec2(uv_x, 1.0f));
			Vec2 markEnd = m_screenSpaceAABB2.GetPointAtUV(Vec2(uv_x, 0.0f));

			m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, m_screenSpaceAABB2.GetBoxWithin(Vec2(uv_x, 1.0f) - Vec2(0.05f, 0.0f), Vec2(uv_x, 1.0f) + Vec2(0.05f, 0.1f)), 9.f, Stringf("%u", i), Rgba8::BLACK, 0.8f);

			m_lineMarksVerts.push_back(Vertex_PCU(markStart, Rgba8::CHELSEA_GREY));
			m_lineMarksVerts.push_back(Vertex_PCU(markEnd, Rgba8::CHELSEA_GREY));
		}
		m_lineMarksVBO = m_renderer.CreateVertexBuffer(m_lineMarksVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "LineMarksVBO", false);
		m_renderer.CopyCPUToGPU(m_lineMarksVerts.data(), m_lineMarksVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_lineMarksVBO);

		m_textVBO = m_renderer.CreateVertexBuffer(m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "TextVBO");
		m_renderer.CopyCPUToGPU(m_textVertices.data(), m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
	}
}

void Timeline::RegisterIndicatorMoveCallbackFunction(std::function<void(float)> function)
{
	m_registeredFunction = function;
}

void Timeline::SetClockToUseAndTimeSpanAndMaybeFPS(Clock& newClockToUse, float newTimeSpan, unsigned int animFPS)
{
	GUARANTEE_OR_DIE(newTimeSpan > 0.0f, "timeSpan <= 0.0f doesn't make sense!");
	m_clockToUse = &newClockToUse;
	m_timeSpan = newTimeSpan;
	m_animFPS = animFPS;

	if (animFPS > 0) {
		if (m_lineMarksVBO) {
			m_lineMarksVerts.clear();
			delete m_lineMarksVBO;
		}
		if (m_textVBO) {
			m_textVertices.clear();
			delete m_textVBO;
		}

		float finalFrameNum = newTimeSpan * (float)animFPS;
		float inv_finalFrameNum = 1.0f / finalFrameNum;
		for (unsigned int i = 0; i <= (unsigned int)finalFrameNum; i += 5) {
			float uv_x = (float)i * inv_finalFrameNum;
			Vec2 markStart = m_screenSpaceAABB2.GetPointAtUV(Vec2(uv_x, 1.0f));
			Vec2 markEnd = m_screenSpaceAABB2.GetPointAtUV(Vec2(uv_x, 0.0f));

			m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, m_screenSpaceAABB2.GetBoxWithin(Vec2(uv_x, 1.0f) - Vec2(0.05f, 0.0f), Vec2(uv_x, 1.0f) + Vec2(0.05f, 0.1f)), 9.f, Stringf("%u", i), Rgba8::BLACK, 0.8f);

			m_lineMarksVerts.push_back(Vertex_PCU(markStart, Rgba8::CHELSEA_GREY));
			m_lineMarksVerts.push_back(Vertex_PCU(markEnd, Rgba8::CHELSEA_GREY));
		}
		m_lineMarksVBO = m_renderer.CreateVertexBuffer(m_lineMarksVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "LineMarksVBO", false);
		m_renderer.CopyCPUToGPU(m_lineMarksVerts.data(), m_lineMarksVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_lineMarksVBO);

		m_textVBO = m_renderer.CreateVertexBuffer(m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "TextVBO");
		m_renderer.CopyCPUToGPU(m_textVertices.data(), m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
	}
}

void Timeline::Update()
{
	GUARANTEE_OR_DIE(m_registeredFunction != nullptr, "No callback function related with the timeline!");
	UpdateFromKeyboard();
	Vec2 newCenter;
	if (m_isIndicatorBeingDragged) {
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 mouseCursorAtOverlayBounds = m_overlay.GetBounds().GetPointAtUV(normalizedCursorPos);
		newCenter = mouseCursorAtOverlayBounds + m_offsetFromMouseClickedToPolyCenter;
		newCenter.x = GetClamped(newCenter.x, m_screenSpaceAABB2.m_mins.x, m_screenSpaceAABB2.m_maxs.x);
		newCenter.y = m_screenSpaceAABB2.m_maxs.y;
		m_indicatorConvexPoly.SetCenterPos(newCenter);
		m_registeredFunction(((newCenter.x - m_screenSpaceAABB2.m_mins.x)/ m_screenSpaceAABB2.GetDimensions().x) * m_timeSpan);
	}
	else {
		float clockTotalSeconds = m_clockToUse->GetTotalSeconds();
		float fraction = clockTotalSeconds / m_timeSpan;
		//GUARANTEE_OR_DIE(fraction <= 1.0f, "Fraction weird");
		newCenter = m_screenSpaceAABB2.GetPointAtUV(Vec2(fraction, 1.0f));
		m_indicatorConvexPoly.SetCenterPos(newCenter);
	}

	//Remake the indicator vbos
	m_indicatorConvexPolyVerts.clear();
	AddVertsForConvexPoly2D(m_indicatorConvexPolyVerts, m_indicatorConvexPoly, Rgba8::MAROON);
	m_renderer.CopyCPUToGPU(m_indicatorConvexPolyVerts.data(), m_indicatorConvexPolyVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_indicatorConvexPolyVBO);
	
	m_indicatorMovingLineVerts.clear();
	m_indicatorMovingLineVerts.push_back(Vertex_PCU(newCenter, Rgba8::GREY));
	m_indicatorMovingLineVerts.push_back(Vertex_PCU(Vec2(newCenter.x, 0.0f), Rgba8::GREY));
	m_renderer.CopyCPUToGPU(m_indicatorMovingLineVerts.data(), m_indicatorMovingLineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_indicatorMovingLineVBO);

}

void Timeline::Render() const
{
	//Bind the model matrix here for indicator
	m_renderer.BindTexture(nullptr);
	
	m_renderer.SetModelConstants();
	m_renderer.DrawVertexBuffer(m_indicatorConvexPolyVBO, (int)m_indicatorConvexPolyVerts.size());
	m_renderer.DrawVertexBuffer(m_indicatorMovingLineVBO, (int)m_indicatorMovingLineVerts.size());

	if (m_lineMarksVBO) {
		m_renderer.DrawVertexBuffer(m_lineMarksVBO, (int)m_lineMarksVerts.size());
	}

	if (m_textVBO) {
		m_renderer.BindTexture(&m_bitmapFont->GetTexture());
		m_renderer.SetModelConstants();
		m_renderer.DrawVertexBuffer(m_textVBO, (int)m_textVertices.size());
	}

	m_renderer.BindTexture(nullptr);
	m_renderer.SetModelConstants(m_modelMatrix, m_backgroundTint);
	m_renderer.DrawVertexBuffer(m_nonTextVBO, 6);
}

Timeline::~Timeline()
{
	delete m_indicatorMovingLineVBO;
	delete m_indicatorConvexPolyVBO;
	delete m_lineMarksVBO;
	delete m_textVBO;
}

void Timeline::UpdateFromKeyboard()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 mouseCursorAtOverlayBounds = m_overlay.GetBounds().GetPointAtUV(normalizedCursorPos);
		if (IsPointInsideConvexPoly2D(mouseCursorAtOverlayBounds, m_indicatorConvexPoly)) {
			m_isIndicatorBeingDragged = true;
			m_offsetFromMouseClickedToPolyCenter = m_indicatorConvexPoly.GetCenterPos() - mouseCursorAtOverlayBounds;
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LMB)) {
		if (m_isIndicatorBeingDragged)
			m_isIndicatorBeingDragged = false;
	}
}
