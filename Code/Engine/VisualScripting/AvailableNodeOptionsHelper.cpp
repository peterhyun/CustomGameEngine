#include "Engine/VisualScripting/AvailableNodeOptionsHelper.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/VisualScripting/PlaceNodeCommand.hpp"
#include "Engine/Visualscripting/PlaceAndConnectNodeCommand.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

AvailableNodeOptionsHelper::AvailableNodeOptionsHelper(BehaviorTreeEditor& editor, Renderer& renderer) : m_dim(HELPER_DIMX, HELPER_DIMY), m_editor(editor), m_renderer(renderer)
{
}

AvailableNodeOptionsHelper::~AvailableNodeOptionsHelper()
{
	delete m_textVBO;
	delete m_outlineVBO;
}

void AvailableNodeOptionsHelper::UpdateFromKeyboard()
{
	Vec2 cursorPos = m_editor.GetCursorPosInEditor();
	AABB2 box(m_posInEditor.x, m_posInEditor.y - m_dim.y, m_posInEditor.x + m_dim.x, m_posInEditor.y);

	const auto& map = m_editor.GetNameAndPrototypeNodeMap();
	float numItemsInv = 1.0f / (float)AVAILABLENODEOPTIONSHELPER_MAXITEMS;

	int itemIdx = 0;
	std::string itemString;
	m_selectedOptionIdx = -1;
	for (auto it = map.begin(); it != map.end(); it++, itemIdx++) {
		AABB2 itemBox = box.GetBoxWithin(Vec2(0.0f, numItemsInv * float(itemIdx)), Vec2(1.0f, numItemsInv * float(itemIdx + 1)));
		if (itemBox.IsPointInside(cursorPos)) {
			m_selectedOptionIdx = itemIdx;
			itemString = it->first;
			break;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		if (m_selectedOptionIdx >= 0) {
			m_editor.SetHelperVisibility(false);
			if (m_nodeToConnectNewNodeTo) {
				m_editor.AddCommandToExecuteQueue(*new PlaceAndConnectNodeCommand(m_editor, itemString, m_posInEditor, *m_nodeToConnectNewNodeTo, m_shouldConnectToTopPort));
				m_nodeToConnectNewNodeTo = nullptr;
			}
			else {
				m_editor.AddCommandToExecuteQueue(*new PlaceNodeCommand(m_editor, itemString, m_posInEditor));
			}
		}
		else {
			m_editor.SetHelperVisibility(false);
		}
	}
}

void AvailableNodeOptionsHelper::SetPositionToFitEditor(const Vec2& newDesiredPos)
{
	//Have to think whether this fits the current screen or not
	AABB2 cameraAABB2 = m_editor.GetMovableCameraAABB2();
	AABB2 desiredHelperAABB2(Vec2(newDesiredPos.x, newDesiredPos.y - HELPER_DIMY), Vec2(newDesiredPos.x + HELPER_DIMX, newDesiredPos.y));
	AABB2 finalHelperAABB2 = cameraAABB2.FixBoxWithinThis(desiredHelperAABB2);
	m_posInEditor = finalHelperAABB2.GetTopLeft();
}

void AvailableNodeOptionsHelper::UpdateInEditor()
{
	UpdateFromKeyboard();
	UpdateOutlineVertsForMe();
	UpdateTextVertsForMe();
}

void AvailableNodeOptionsHelper::Render() const
{
	Renderer& renderer = m_editor.GetRefToRenderer();
	if (m_outlineVBO) {
		renderer.BindShader(nullptr);
		renderer.BindTexture(nullptr);
		renderer.DrawVertexBuffer(m_outlineVBO, (int)m_outlineVerts.size());
	}

	if (m_textVBO) {
		BitmapFont* bitmapFont = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		GUARANTEE_OR_DIE(bitmapFont != nullptr, "Check BitmapFont name");
		renderer.BindTexture(&bitmapFont->GetTexture());
		renderer.DrawVertexBuffer(m_textVBO, (int)m_textVerts.size());
	}
}

void AvailableNodeOptionsHelper::ConnectNextNewNodeToThisNode(BehaviorTreeNode& thisNode, bool shouldConnectToTopPort)
{
	m_nodeToConnectNewNodeTo = &thisNode;
	m_shouldConnectToTopPort = shouldConnectToTopPort;
}

void AvailableNodeOptionsHelper::UpdateTextVertsForMe()
{
	m_textVerts.clear();

	AABB2 box(m_posInEditor.x, m_posInEditor.y - m_dim.y, m_posInEditor.x + m_dim.x, m_posInEditor.y);

	Renderer& renderer = m_editor.GetRefToRenderer();
	BitmapFont* bitmapFont = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	GUARANTEE_OR_DIE(bitmapFont != nullptr, "Check BitmapFont name");

	const auto& map = m_editor.GetNameAndPrototypeNodeMap();
	int itemIdx = 0;
	float numItemsInv = 1.0f / (float)AVAILABLENODEOPTIONSHELPER_MAXITEMS;
	for (auto it = map.begin(); it != map.end(); it++, itemIdx++) {
		if (itemIdx != m_selectedOptionIdx) {
			bitmapFont->AddVertsForTextInBox2D(m_textVerts, box.GetBoxWithin(Vec2(0.0f, numItemsInv * float(itemIdx)), Vec2(1.0f, numItemsInv * float(itemIdx + 1))), 40.0f, it->first, Rgba8::WHITE, FONT_ASPECT);
		}
		else {
			bitmapFont->AddVertsForTextInBox2D(m_textVerts, box.GetBoxWithin(Vec2(0.0f, numItemsInv * float(itemIdx)), Vec2(1.0f, numItemsInv * float(itemIdx + 1))), 40.0f, it->first, Rgba8::BLUEGREEN, FONT_ASPECT);
		}
	}

	if (m_textVBO == nullptr) {
		m_textVBO = renderer.CreateVertexBuffer(m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "Text VBO for node helper");
	}
	renderer.CopyCPUToGPU(m_textVerts.data(), m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
}

void AvailableNodeOptionsHelper::UpdateOutlineVertsForMe()
{
	m_outlineVerts.clear();

	AABB2 box(m_posInEditor.x, m_posInEditor.y - m_dim.y, m_posInEditor.x + m_dim.x, m_posInEditor.y);
	AddVertsForAABB2(m_outlineVerts, box, Rgba8::BLACK);

	if (m_selectedOptionIdx != -1) {
		float numItemsInv = 1.0f / (float)AVAILABLENODEOPTIONSHELPER_MAXITEMS;
		AABB2 itemBox = box.GetBoxWithin(Vec2(0.0f, numItemsInv * float(m_selectedOptionIdx)), Vec2(1.0f, numItemsInv * float(m_selectedOptionIdx + 1)));
		AddVertsForAABB2(m_outlineVerts, itemBox, Rgba8::PASTEL_BLUE);
	}

	Renderer& renderer = m_editor.GetRefToRenderer();
	if (m_outlineVBO == nullptr) {
		m_outlineVBO = renderer.CreateVertexBuffer(m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "Outline VBO for node helper");
	}
	renderer.CopyCPUToGPU(m_outlineVerts.data(), m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_outlineVBO);
}
