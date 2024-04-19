#include "Engine/UI/Button.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"

Button::Button(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const std::string& buttonText, float buttonTextSize, const Vec2& textAlignment, const Rgba8& textNormalColor, const Rgba8& textHoveredColor, const Rgba8& buttonNormalColor, const Rgba8& buttonHoveredColor, const std::string& name)
	: TextBox(rendererToUse, overlay, screenSpaceAABB2, buttonText, buttonTextSize, textAlignment, textNormalColor, buttonNormalColor, name), m_textHoveredColor(textHoveredColor), m_buttonHoveredColor(buttonHoveredColor)
{
	m_widgetType = WidgetType::BUTTON;
}

void Button::RegisterCallbackFunction(std::function<void()> function)
{
	m_registeredFunction = function;
}

Button::~Button()
{
}

/*
void Button::OnHovered()
{
}

void Button::OnFocused()
{
	m_isFocused = true;
}

void Button::OnClick()
{
}
*/

void Button::OnClick()
{
	if (m_registeredFunction) {
		m_registeredFunction();
	}
}

void Button::SetIsFocused(bool isFocused)
{
	m_isFocused = isFocused;
}

void Button::Update()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 mouseCursorAtOverlayBounds = m_overlay.GetBounds().GetPointAtUV(normalizedCursorPos);
		if (m_screenSpaceAABB2.IsPointInside(mouseCursorAtOverlayBounds)) {
			OnClick();
		}
	}
}

void Button::Render() const
{
	m_renderer.BindTexture(nullptr);
	if (m_borderVBO) {
		m_renderer.SetModelConstants(m_modelMatrix, m_borderTint);
		m_renderer.DrawVertexBuffer(m_borderVBO, 6);
	}
	if (m_isFocused) {
		m_renderer.SetModelConstants(m_modelMatrix, m_buttonHoveredColor);
		m_renderer.DrawVertexBuffer(m_nonTextVBO, 6);
	}
	else {
		m_renderer.SetModelConstants(m_modelMatrix, m_backgroundTint);
		m_renderer.DrawVertexBuffer(m_nonTextVBO, 6);
	}

	if (m_isFocused) {
		m_renderer.SetModelConstants(Mat44(), m_textHoveredColor);
	}
	else {
		m_renderer.SetModelConstants(Mat44(), m_textTint);
	}
	m_renderer.BindTexture(&m_bitmapFont->GetTexture());
	m_renderer.DrawVertexBuffer(m_textVBO, (int)m_textVertices.size());
}