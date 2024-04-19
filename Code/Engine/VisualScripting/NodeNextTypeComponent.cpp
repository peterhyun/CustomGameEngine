#include "Engine/VisualScripting/NodeTextTypeBarComponent.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

NodeTextTypeBarComponent::NodeTextTypeBarComponent(const BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs, float textCellHeight) : m_ownerNode(ownerNode), m_nodeUVMins(nodeUVMins), m_nodeUVMaxs(nodeUVMaxs), m_textCellHeight(textCellHeight)
{
 	m_caretStopwatch = new Stopwatch(0.5f);
	m_caretStopwatch->Start();
}

NodeTextTypeBarComponent::~NodeTextTypeBarComponent()
{
	delete m_caretStopwatch;
}

void NodeTextTypeBarComponent::UpdateInEditor()
{
	UpdateFromKeyboard();

	if (m_isActive == false) {
		m_isCaretVisible = false;
		return;
	}

	while (m_caretStopwatch->DecrementDurationIfElapsed()) {
		m_isCaretVisible = !m_isCaretVisible;
	}
}

void NodeTextTypeBarComponent::SetText(const std::string& text)
{
	m_inputText = text;
}

void NodeTextTypeBarComponent::AddOutlineVertsForMe(std::vector<Vertex_PCU>& out_verts)
{
	AABB2 componentOutline = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	AddVertsForAABB2(out_verts, componentOutline, Rgba8::WHITE);
}

void NodeTextTypeBarComponent::AddTextVertsForMe(const BitmapFont& bitmapFont, std::vector<Vertex_PCU>& out_verts)
{
}

void NodeTextTypeBarComponent::UpdateFromKeyboard()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2();
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		const BehaviorTreeEditor* editor = m_ownerNode.GetConstPtrToEditor();
		GUARANTEE_OR_DIE(editor != nullptr, "m_ownerNode == nullptr");
		Vec2 pointOnScreen = editor->GetCursorPosInEditor();
		if (IsPointInsideAABB2D(pointOnScreen, screenAABB2)) {
			m_isActive = true;
		}
		else {
			m_isActive = false;
		}
	}

	if (m_isActive == false) {
		return;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC)) {
		m_isActive = false;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER)) {
		if (m_inputText.size() > 0) {
			bool returnResult = false;
			if (m_enterKeyCallbackFunction) {
				returnResult = m_enterKeyCallbackFunction(m_inputText);
			}
			if (returnResult == true) {
				m_inputText.clear();
				m_caretPosition = 0;
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW)) {
		if (m_caretPosition > 0) {
			m_caretPosition--;
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW)) {
		if (m_caretPosition < m_inputText.size()) {
			m_caretPosition++;
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_BACKSPACE)) {
		if (m_caretPosition > 0) {
			m_inputText.erase(m_caretPosition - 1, 1);
			m_caretPosition--;
		}
	}
}
