#pragma once
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/Math/Vec2.hpp"
#include <string>

class BehaviorTreeEditor;
class BehaviorTreeNode;

class DeleteNodeCommand : public Command {
public:
	DeleteNodeCommand(BehaviorTreeEditor& editor, BehaviorTreeNode& node);
	virtual void Execute() override;
	virtual void Undo() override;

private:
	BehaviorTreeEditor& m_editor;
	BehaviorTreeNode* const m_node = nullptr;
	BehaviorTreeNode* m_parentNodeOfDeletedNode = nullptr;

	Vec2 m_posWhenItGotDeleted;
};