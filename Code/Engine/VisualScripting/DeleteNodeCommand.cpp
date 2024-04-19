#include "Engine/VisualScripting/DeleteNodeCommand.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

DeleteNodeCommand::DeleteNodeCommand(BehaviorTreeEditor& editor, BehaviorTreeNode& node) : m_editor(editor), m_node(&node), m_posWhenItGotDeleted(node.GetCurrentPos())
{
}

void DeleteNodeCommand::Execute()
{
	m_parentNodeOfDeletedNode = m_node->GetParentNode();
	if (m_parentNodeOfDeletedNode) {
		m_parentNodeOfDeletedNode->RemoveNodeFromChildren(*m_node);
	}
	m_editor.DeleteNodeToGarbageCollection(*m_node);
}

void DeleteNodeCommand::Undo()
{
	m_editor.BringBackNodeFromGarbageCollection(*m_node);

	if (m_parentNodeOfDeletedNode) {
		m_parentNodeOfDeletedNode->AddNodeToChildren(*m_node);
	}
}