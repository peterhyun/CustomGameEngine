#include "Engine/VisualScripting/NodeDropDownComponent.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"

NodeDropDownComponent::NodeDropDownComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVButtonMins, const Vec2& nodeUVButtonMaxs, const std::vector<std::string>& options, const Vec2& eachOptionAABB2Dimension)
	: NodeHelperComponent(ownerNode, nodeUVButtonMins, nodeUVButtonMaxs), m_options(options), m_eachOptionAABB2Dim(eachOptionAABB2Dimension), m_inv_numOptions(1.0f / (float)options.size())
{
}

NodeDropDownComponent::~NodeDropDownComponent()
{
}

void NodeDropDownComponent::UpdateInEditor()
{
	//Update m_fullBox every frame
	AABB2 buttonScreenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);

	Vec2 fullOptionBoxSize(m_eachOptionAABB2Dim.x, m_eachOptionAABB2Dim.y * (float)m_options.size());
	m_fullBox = AABB2(Vec2(), fullOptionBoxSize);

	float xOffset = (buttonScreenAABB2.GetDimensions().x - fullOptionBoxSize.x) * 0.5f;
	m_fullBox.Translate(Vec2(buttonScreenAABB2.m_maxs.x - m_eachOptionAABB2Dim.x - xOffset, buttonScreenAABB2.m_mins.y - fullOptionBoxSize.y));
	
	m_hoveredOptionIdx = -1;
	if (m_isActive) {
		Vec2 pointOnScreen = m_ownerNode.GetConstPtrToEditor()->GetCursorPosInEditor();
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

		Vec2 pointOnScreen = m_ownerNode.GetConstPtrToEditor()->GetCursorPosInEditor();
		if (IsPointInsideAABB2D(pointOnScreen, buttonScreenAABB2)) {
			m_isActive = !m_isActive;
		}
		else {
			m_isActive = false;
		}
	}

	UpdateOutlineVertsForMe();
	UpdateTextVertsForMe();
}

int NodeDropDownComponent::GetSelectedOptionIdx() const
{
	return m_selectedOptionIdx;
}

void NodeDropDownComponent::SetSelectedOptionIdx(int newIdx)
{
	GUARANTEE_OR_DIE(newIdx >= -1 && newIdx < m_options.size(), std::string("newIdx %d invalid", newIdx));
	m_selectedOptionIdx = newIdx;
}

void NodeDropDownComponent::UpdateOutlineVertsForMe()
{
	m_outlineVerts.clear();

	AABB2 buttonScreenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	AddVertsForAABB2(m_outlineVerts, buttonScreenAABB2, Rgba8::GREY);

	if (m_isActive) {
		AddVertsForAABB2(m_outlineVerts, m_fullBox, Rgba8::GREY);
		if (m_hoveredOptionIdx >= 0) {
			AABB2 hoveredOptionBox = m_fullBox.GetBoxWithin(Vec2(0.0f, (1.0f - m_inv_numOptions * float(m_hoveredOptionIdx + 1))), Vec2(1.0f, (1.0f - m_inv_numOptions * (float)m_hoveredOptionIdx)));
			AddVertsForAABB2(m_outlineVerts, hoveredOptionBox, Rgba8::PASTEL_BLUE);
		}
	}

	Renderer& renderer = m_ownerNode.GetConstPtrToEditor()->GetRefToRenderer();
	if (m_outlineVBO == nullptr) {
		m_outlineVBO = renderer.CreateVertexBuffer(m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "OutlineVBO for NodeDropDownComponent");
	}
	renderer.CopyCPUToGPU(m_outlineVerts.data(), m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_outlineVBO);
}

void NodeDropDownComponent::UpdateTextVertsForMe()
{
	m_textVerts.clear();

	Renderer& renderer = m_ownerNode.GetConstPtrToEditor()->GetRefToRenderer();
	AABB2 buttonScreenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	BitmapFont* bitmapFont = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	GUARANTEE_OR_DIE(bitmapFont != nullptr, "Check BitmapFont file");
	if (m_selectedOptionIdx >= 0) {
		bitmapFont->AddVertsForTextInBox2D(m_textVerts, buttonScreenAABB2, 15.0f, m_options[m_selectedOptionIdx], Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
	}
	else {
		bitmapFont->AddVertsForTextInBox2D(m_textVerts, buttonScreenAABB2, 15.0f, "Select Operator!", Rgba8::WHITE, 0.8f, Vec2(0.5f, 0.5f));
	}

	if (m_isActive) {
		for (int i = 0; i < m_options.size(); i++) {
			bitmapFont->AddVertsForTextInBox2D(m_textVerts, m_fullBox.GetBoxWithin(Vec2(0.0f, (1.0f - m_inv_numOptions * float(i + 1))), Vec2(1.0f, (1.0f - m_inv_numOptions * (float)i))), 15.0f, m_options[i], Rgba8::KENDALL_CHARCOAL, 0.8f);
		}
	}

	if (m_textVBO == nullptr) {
		m_textVBO = renderer.CreateVertexBuffer(m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "TextVBO for NodeDropDownComponent");
	}
	renderer.CopyCPUToGPU(m_textVerts.data(), m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
}

void NodeDropDownComponent::ResetSelectedOptionIdx()
{
	m_selectedOptionIdx = -1;
}