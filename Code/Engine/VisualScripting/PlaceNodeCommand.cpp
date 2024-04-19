#include "Engine/VisualScripting/PlaceNodeCommand.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

PlaceNodeCommand::PlaceNodeCommand(BehaviorTreeEditor& editor, const std::string& nodeName, const Vec2& pos) : m_editor(editor), m_nodeName(nodeName), m_pos(pos)
{
}

void PlaceNodeCommand::Execute()
{
	if (m_node == nullptr) {
		m_node = m_editor.PlaceNodeAtPos(m_nodeName, m_pos);
		m_node->SetBehaviorTreeEditor(m_editor);
	}
	else {
		m_editor.BringBackNodeFromGarbageCollection(*m_node);
	}
}

void PlaceNodeCommand::Undo()
{
	GUARANTEE_OR_DIE(m_node != nullptr, "m_node should not be a nullptr when calling PlaceNodeCommand::Undo()");
	m_editor.DeleteNodeToGarbageCollection(*m_node);
}
