#include "Engine/VisualScripting/PlaceAndConnectNodeCommand.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

PlaceAndConnectNodeCommand::PlaceAndConnectNodeCommand(BehaviorTreeEditor& editor, const std::string& nodeName, const Vec2& pos, BehaviorTreeNode& nodeToConnectTo, bool shouldConnectToTopPort)
	: m_editor(editor), m_nodeName(nodeName), m_pos(pos), m_nodeToConnectTo(nodeToConnectTo), m_shouldConnectToTopPort(shouldConnectToTopPort)
{
}

void PlaceAndConnectNodeCommand::Execute()
{
	if (m_newlyPlacedNode == nullptr) {
		m_newlyPlacedNode = m_editor.PlaceNodeAtPos(m_nodeName, m_pos);
		m_newlyPlacedNode->SetBehaviorTreeEditor(m_editor);
	}
	else {
		m_editor.BringBackNodeFromGarbageCollection(*m_newlyPlacedNode);
	}

	if (m_shouldConnectToTopPort) {
		m_newlyPlacedNode->AddNodeToChildren(m_nodeToConnectTo);
		m_newlyPlacedNode->ReorderChildNodes();
	}
	else {
		m_nodeToConnectTo.AddNodeToChildren(*m_newlyPlacedNode);
		m_newlyPlacedNode->ReorderChildNodes();
	}
}

void PlaceAndConnectNodeCommand::Undo()
{
	GUARANTEE_OR_DIE(m_newlyPlacedNode != nullptr, "m_newlyPlacedNode should not be a nullptr when calling PlaceAndConnectNodeCommand::Undo()");

	if (m_shouldConnectToTopPort) {
		m_newlyPlacedNode->RemoveNodeFromChildren(m_nodeToConnectTo);
	}
	else {
		m_nodeToConnectTo.RemoveNodeFromChildren(*m_newlyPlacedNode);
	}
	m_editor.DeleteNodeToGarbageCollection(*m_newlyPlacedNode);
}
