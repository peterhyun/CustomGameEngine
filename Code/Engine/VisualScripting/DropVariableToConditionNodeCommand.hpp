#pragma once
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/Math/Vec2.hpp"
#include <string>

class BehaviorTreeEditor;
class ConditionNode;

class DropVariableToConditionNodeCommand : public Command {
public:
	DropVariableToConditionNodeCommand(ConditionNode& node, const std::string& variableName);
	virtual void Execute() override;
	virtual void Undo() override;

private:
	ConditionNode& m_node;
	const std::string& m_variableName;
	std::string m_previousTextBeforeDropping;
};