#pragma once
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/Math/Vec2.hpp"

class AddConnectionCommand : public Command {
public:
	AddConnectionCommand(class BehaviorTreeEditor& editor, class BehaviorTreeNode& parentNode, class BehaviorTreeNode& childNode);
	virtual void Execute() override;
	virtual void Undo() override;

private:
	BehaviorTreeEditor& m_editor;
	BehaviorTreeNode& m_parentNode;
	BehaviorTreeNode& m_childNode;
	Vec2 m_newPos;
	Vec2 m_prevPos;
};