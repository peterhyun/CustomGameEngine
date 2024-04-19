#include "Engine/UI/DropDownComponent.hpp"
#include "Engine/UI/Widget.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"

DropDownComponent::DropDownComponent(Renderer& renderer, const std::vector<std::string>& options, const Widget& ownerWidget, const AABB2& buttonScreenAABB2, bool isOpenByDefault, bool shouldRenderUp, const Vec2& eachOptionAABB2Dim)
	: m_options(options), m_ownerWidget(ownerWidget), m_buttonScreenAABB2(buttonScreenAABB2), m_isOpen(isOpenByDefault), m_renderer(renderer), m_eachOptionAABB2Dim(eachOptionAABB2Dim), m_shouldRenderUp(shouldRenderUp), m_inv_numOptions(1.0f / (float)options.size())
{
	m_bitmapFont = m_renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	Vec2 fullBoxSize(m_eachOptionAABB2Dim.x, m_eachOptionAABB2Dim.y * (float)m_options.size());
	m_fullBox = AABB2(Vec2(), fullBoxSize);
	if (m_shouldRenderUp) {
		m_fullBox.Translate(Vec2(m_buttonScreenAABB2.m_maxs.x - m_eachOptionAABB2Dim.x, m_buttonScreenAABB2.m_maxs.y));
	}
	else {
		m_fullBox.Translate(Vec2(m_buttonScreenAABB2.m_maxs.x - m_eachOptionAABB2Dim.x, m_buttonScreenAABB2.m_mins.y - fullBoxSize.y));
	}
}

DropDownComponent::~DropDownComponent()
{
}

void DropDownComponent::Update()
{
	m_hoveredOptionIdx = -1;
	if (m_isOpen) {
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 pointOnScreen = m_ownerWidget.GetConstRefToOverlay().GetBounds().GetPointAtUV(normalizedCursorPos);
		float inv_numOptions = 1.0f / (float)m_options.size();
		for (int i = 0; i < m_options.size(); i++) {
			AABB2 optionBox = m_fullBox.GetBoxWithin(Vec2(0.0f, 1.0f - (float)(i + 1) * inv_numOptions), Vec2(1.0f, 1.0f - (float)i * inv_numOptions));
			if (IsPointInsideAABB2D(pointOnScreen, optionBox)) {
				m_hoveredOptionIdx = i;
				break;
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		if (m_hoveredOptionIdx >= 0) {
			m_selectedOptionIdx = m_hoveredOptionIdx;
		}

		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 pointOnScreen = m_ownerWidget.GetConstRefToOverlay().GetBounds().GetPointAtUV(normalizedCursorPos);
		if (IsPointInsideAABB2D(pointOnScreen, m_buttonScreenAABB2)) {
			m_isOpen = !m_isOpen;
		}
		else {
			m_isOpen = false;
		}
	}
}

void DropDownComponent::Render() const
{
	std::vector<Vertex_PCU> outlineVerts;
	std::vector<Vertex_PCU> textVerts;

	if (m_selectedOptionIdx >= 0) {
		m_bitmapFont->AddVertsForTextInBox2D(textVerts, m_buttonScreenAABB2, 15.0f, m_options[m_selectedOptionIdx], Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
	}
	else {
		m_bitmapFont->AddVertsForTextInBox2D(textVerts, m_buttonScreenAABB2, 15.0f, "INVALID", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
	}
	AddVertsForAABB2(outlineVerts, m_buttonScreenAABB2, Rgba8::GREY);

	if (m_isOpen) {
		if (m_shouldRenderUp) {
			AddVertsForAABB2(outlineVerts, m_fullBox, Rgba8::GREY);
		}
		else {
			AddVertsForAABB2(outlineVerts, m_fullBox, Rgba8::GREY);
		}
		if (m_hoveredOptionIdx >= 0) {
			AABB2 hoveredOptionBox = m_fullBox.GetBoxWithin(Vec2(0.0f, (1.0f - m_inv_numOptions * float(m_hoveredOptionIdx + 1))), Vec2(1.0f, (1.0f - m_inv_numOptions * (float)m_hoveredOptionIdx)));
			AddVertsForAABB2(outlineVerts, hoveredOptionBox, Rgba8::PASTEL_BLUE);
		}

		for (int i = 0; i < m_options.size(); i++) {
			m_bitmapFont->AddVertsForTextInBox2D(textVerts, m_fullBox.GetBoxWithin(Vec2(0.0f, (1.0f - m_inv_numOptions * float(i + 1))), Vec2(1.0f, (1.0f - m_inv_numOptions * (float)i))), 15.0f, m_options[i], Rgba8::KENDALL_CHARCOAL, 0.8f);
		}

	}

	m_renderer.BindTexture(nullptr);
	m_renderer.SetModelConstants();
	m_renderer.DrawVertexArray((int)outlineVerts.size(), outlineVerts.data());
	m_renderer.BindTexture(&m_bitmapFont->GetTexture());
	m_renderer.DrawVertexArray((int)textVerts.size(), textVerts.data());
}

std::string DropDownComponent::GetSelectedOptionString() const
{
	if (m_selectedOptionIdx >= 0) {
		return m_options[m_selectedOptionIdx];
	}
	else {
		return "INVALID";
	}
}

void DropDownComponent::ResetSelectedOptionIdx()
{
	m_selectedOptionIdx = -1;
}
