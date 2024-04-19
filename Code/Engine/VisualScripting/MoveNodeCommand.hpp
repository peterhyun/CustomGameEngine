#pragma once
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/Math/Vec2.hpp"

class MoveNodeCommand : public Command {
public:
	MoveNodeCommand(class BehaviorTreeEditor& editor, class BehaviorTreeNode& node, const Vec2& newPos, const Vec2& prevPos);
	virtual void Execute() override;
	virtual void Undo() override;

private:
	BehaviorTreeEditor& m_editor;
	BehaviorTreeNode& m_node;
	Vec2 m_newPos;
	Vec2 m_prevPos;
};