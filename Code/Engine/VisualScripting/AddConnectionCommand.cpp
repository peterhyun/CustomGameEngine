#include "Engine/VisualScripting/AddConnectionCommand.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"

AddConnectionCommand::AddConnectionCommand(BehaviorTreeEditor& editor, BehaviorTreeNode& parentNode, BehaviorTreeNode& childNode): m_editor(editor), m_parentNode(parentNode), m_childNode(childNode)
{
}

void AddConnectionCommand::Execute()
{
	m_parentNode.AddNodeToChildren(m_childNode);
	m_parentNode.ReorderChildNodes();
}

void AddConnectionCommand::Undo()
{
	m_parentNode.RemoveNodeFromChildren(m_childNode);
}