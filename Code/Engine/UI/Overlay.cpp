#include "Engine/UI/Overlay.hpp"
#include "Engine/UI/Button.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

Overlay::Overlay(Renderer& renderer, const AABB2& clientSpaceAABB2, const Rgba8& tint) : m_renderer(renderer), m_bounds(clientSpaceAABB2)
{
	AddVertsForAABB2(m_quadVerts, m_bounds, tint);
	m_quadVBO = m_renderer.CreateVertexBuffer(m_quadVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "Overlay Quad VBO");
	m_renderer.CopyCPUToGPU(m_quadVerts.data(), m_quadVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_quadVBO);
}

Overlay::~Overlay()
{
	delete m_quadVBO;

	for (int i = 0; i < m_widgets.size(); i++) {
		delete m_widgets[i];
	}
}

void Overlay::Update()
{
	const std::vector<Widget*>& buttonsList = m_widgetsByType[(int)WidgetType::BUTTON];

	/*
	//Always default selects the first button
	if (m_selectedButtonIdx == -1 && !buttonsList.empty()) {
		m_selectedButtonIdx = 0;
		dynamic_cast<Button *>(buttonsList[0])->SetIsFocused(true);
	}
	*/

	for (int i = 0; i < m_widgets.size(); i++) {
		if (m_widgets[i] && m_widgets[i]->m_isDisabled == false) {
			m_widgets[i]->Update();
		}
	}

	Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
	Vec2 mouseCursorAtOverlayBounds = m_bounds.GetPointAtUV(normalizedCursorPos);
	
	for (int i = 0; i < buttonsList.size(); i++) {
		if (buttonsList[i] && buttonsList[i]->m_isDisabled == false) {
			dynamic_cast<Button*>(buttonsList[i])->SetIsFocused(false);
			if (buttonsList[i]->m_screenSpaceAABB2.IsPointInside(mouseCursorAtOverlayBounds)) {
				/*
				if (m_selectedButtonIdx != -1) {
					dynamic_cast<Button*>(buttonsList[m_selectedButtonIdx])->SetIsFocused(false);
				}
				*/
				m_selectedButtonIdx = i;
				dynamic_cast<Button*>(buttonsList[m_selectedButtonIdx])->SetIsFocused(true);
			}
		}
	}
}

void Overlay::Render() const
{
	m_renderer.SetBlendMode(BlendMode::ALPHA);
	for (int i = 0; i < m_widgets.size(); i++) {
		if (m_widgets[i] && m_widgets[i]->m_isDisabled == false) {
			m_widgets[i]->Render();
		}
	}
	m_renderer.BindTexture(nullptr);
	m_renderer.DrawVertexBuffer(m_quadVBO, 6);
}

AABB2 Overlay::GetBounds() const
{
	return m_bounds;
}

void Overlay::AddWidget(Widget& widget)
{
	if (m_bounds.IsAABB2Inside(widget.m_screenSpaceAABB2) == false) {
		ERROR_AND_DIE("Invalid AABB2 setting for Overlay::AddWidget()!");
	}

	for (auto m_widget : m_widgets) {
		if (m_widget->m_name == widget.m_name) {
			ERROR_AND_DIE(Stringf("Widget with duplicate name %s already exists!", widget.m_name.c_str()));
		}
	}

	m_widgets.push_back(&widget);

	if (widget.m_widgetType == WidgetType::INVALID) {
		ERROR_AND_DIE("WidgetType should not be INVALID");
	}

	m_widgetsByType[(int)widget.m_widgetType].push_back(&widget);
}

void Overlay::FocusOnPreviousButton()
{
	const std::vector<Widget*>& buttonsList = m_widgetsByType[(int)WidgetType::BUTTON];
	if (m_selectedButtonIdx <= 0) {
		return;
	}
	dynamic_cast<Button*>(buttonsList[m_selectedButtonIdx])->SetIsFocused(false);
	m_selectedButtonIdx--;
	dynamic_cast<Button*>(buttonsList[m_selectedButtonIdx])->SetIsFocused(true);
}

void Overlay::FocusOnNextButton()
{
	const std::vector<Widget*>& buttonsList = m_widgetsByType[(int)WidgetType::BUTTON];
	if (m_selectedButtonIdx >= buttonsList.size() - 1) {
		return;
	}
	dynamic_cast<Button*>(buttonsList[m_selectedButtonIdx])->SetIsFocused(false);
	m_selectedButtonIdx++;
	dynamic_cast<Button*>(buttonsList[m_selectedButtonIdx])->SetIsFocused(true);
}

void Overlay::ClickFocusedButton()
{
	const std::vector<Widget*>& buttonsList = m_widgetsByType[(int)WidgetType::BUTTON];
	if (m_selectedButtonIdx >= buttonsList.size() || m_selectedButtonIdx < 0)
		return;
	dynamic_cast<Button*>(buttonsList[m_selectedButtonIdx])->OnClick();
}

Widget* Overlay::GetWidgetFromName(const std::string& name) const
{
	for (auto widget : m_widgets) {
		if (widget->m_name == name) {
			return widget;
		}
	}
	return nullptr;
}

void Overlay::SetWhetherWidgetIsDisabledFromName(const std::string& name, bool isDisabled)
{
	for (auto widget : m_widgets) {
		if (widget->m_name == name) {
			widget->m_isDisabled = isDisabled;
		}
	}
}
