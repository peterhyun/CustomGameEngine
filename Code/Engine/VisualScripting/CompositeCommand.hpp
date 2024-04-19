#pragma once
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/Math/Vec2.hpp"
#include <string>
#include <vector>

class BehaviorTreeEditor;
class BehaviorTreeNode;

class CompositeCommand : public Command {
public:
	CompositeCommand(const std::vector<Command*>& commands);
	virtual ~CompositeCommand();
	virtual void Execute() override;
	virtual void Undo() override;

private:
	const std::vector<Command*> m_commands;
};