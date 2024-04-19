#include "Engine/UI/PopupPlayer.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

PopupPlayer::PopupPlayer(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Rgba8& tint, const std::string& name)
	: Widget(rendererToUse, overlay, screenSpaceAABB2, tint, name)
{
	m_widgetType = WidgetType::POPUPPLAYER;
}

PopupPlayer::~PopupPlayer()
{
}

void PopupPlayer::Update()
{
	UpdateFromKeyboard();
	if (m_isDragged) {
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		AABB2 overlayBounds = m_overlay.GetBounds();
		Vec2 mouseCursorAtOverlayBounds = overlayBounds.GetPointAtUV(normalizedCursorPos);
		m_screenSpaceAABB2.SetCenter(mouseCursorAtOverlayBounds + m_offsetFromClickedMouseCursorToCenter);

		if (DoesAABB2FitInOtherAABB2(m_screenSpaceAABB2, overlayBounds) == false) {
			m_screenSpaceAABB2 = overlayBounds.FixBoxWithinThis(m_screenSpaceAABB2);
			m_offsetFromClickedMouseCursorToCenter = m_screenSpaceAABB2.GetCenter() - mouseCursorAtOverlayBounds;
			return;
		}

		m_nonTextVerts.clear();
		delete m_nonTextVBO;

		AddVertsForAABB2(m_nonTextVerts, m_screenSpaceAABB2);
		m_nonTextVBO = m_renderer.CreateVertexBuffer(m_nonTextVerts.size(), sizeof(Vertex_PCU), "Widget Quad VBO");
		m_renderer.CopyCPUToGPU(m_nonTextVerts.data(), 6 * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_nonTextVBO);
	}
}

void PopupPlayer::Render() const
{
	m_renderer.SetModelConstants(m_modelMatrix, m_backgroundTint);
	m_renderer.DrawVertexBuffer(m_nonTextVBO, 6);

	if (m_popupRenderCallbackFunction)
		m_popupRenderCallbackFunction();
}

void PopupPlayer::RegisterPopupRenderCallbackFunction(std::function<void()> function)
{
	m_popupRenderCallbackFunction = function;
}

void PopupPlayer::UpdateFromKeyboard()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 mouseCursorAtOverlayBounds = m_overlay.GetBounds().GetPointAtUV(normalizedCursorPos);
		if (m_screenSpaceAABB2.IsPointInside(mouseCursorAtOverlayBounds)) {
			m_isDragged = true;
			m_offsetFromClickedMouseCursorToCenter = m_screenSpaceAABB2.GetCenter() - mouseCursorAtOverlayBounds;
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LMB)) {
		m_isDragged = false;
	}
}
