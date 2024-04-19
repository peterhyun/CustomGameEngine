#include "Engine/VisualScripting/MoveNodeCommand.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"

MoveNodeCommand::MoveNodeCommand(BehaviorTreeEditor& editor, BehaviorTreeNode& node, const Vec2& newPos, const Vec2& prevPos) : m_editor(editor), m_node(node), m_newPos(newPos), m_prevPos(prevPos)
{
}

void MoveNodeCommand::Execute()
{
	m_node.MoveNode(m_newPos);
	BehaviorTreeNode* parentNode = m_node.GetParentNode();
	if (parentNode) {
		parentNode->ReorderChildNodes();
	}
}

void MoveNodeCommand::Undo()
{
	m_node.MoveNode(m_prevPos);
}
