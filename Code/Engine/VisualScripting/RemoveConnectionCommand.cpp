#include "Engine/VisualScripting/RemoveConnectionCommand.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"

RemoveConnectionCommand::RemoveConnectionCommand(BehaviorTreeEditor& editor, BehaviorTreeNode& parentNode, BehaviorTreeNode& childNode) : m_editor(editor), m_parentNode(parentNode), m_childNode(&childNode)
{
}

RemoveConnectionCommand::RemoveConnectionCommand(BehaviorTreeEditor& editor, BehaviorTreeNode& parentNode, const std::list<BehaviorTreeNode*>& childNodes) : m_editor(editor), m_parentNode(parentNode), m_childNodes(childNodes)
{
}

void RemoveConnectionCommand::Execute()
{
	if (m_childNodes.size() == 0) {
		m_parentNode.RemoveNodeFromChildren(*m_childNode);
	}
	else {
		for (BehaviorTreeNode* childNode : m_childNodes) {
			m_parentNode.RemoveNodeFromChildren(*childNode);
		}
	}
}

void RemoveConnectionCommand::Undo()
{
	if (m_childNodes.size() == 0) {
		m_parentNode.AddNodeToChildren(*m_childNode);
	}
	else {
		for (BehaviorTreeNode* childNode : m_childNodes) {
			m_parentNode.AddNodeToChildren(*childNode);
		}
	}
}