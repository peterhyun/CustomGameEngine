#pragma once
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/Math/Vec2.hpp"
#include <list>

class RemoveConnectionCommand : public Command {
public:
	RemoveConnectionCommand(class BehaviorTreeEditor& editor, class BehaviorTreeNode& parentNode, class BehaviorTreeNode& childNode);
	RemoveConnectionCommand(class BehaviorTreeEditor& editor, class BehaviorTreeNode& parentNode, const std::list<BehaviorTreeNode*>& childNodes);
	virtual void Execute() override;
	virtual void Undo() override;

private:
	BehaviorTreeEditor& m_editor;
	BehaviorTreeNode& m_parentNode;
	BehaviorTreeNode* m_childNode = nullptr;
	const std::list<BehaviorTreeNode*> m_childNodes;
	Vec2 m_newPos;
	Vec2 m_prevPos;
};