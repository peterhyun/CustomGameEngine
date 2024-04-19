#include "Engine/VisualScripting/DropVariableToConditionNodeCommand.hpp"
#include "Engine/VisualScripting/ConditionNode.hpp"
#include "Engine/VisualScripting/NodeTextTypeBarComponent.hpp"

DropVariableToConditionNodeCommand::DropVariableToConditionNodeCommand(ConditionNode& node, const std::string& variableName)
	: m_node(node), m_variableName(variableName)
{
}

void DropVariableToConditionNodeCommand::Execute()
{
	m_previousTextBeforeDropping = m_node.m_leftComponent->GetText();
	m_node.m_leftComponent->SetText(m_variableName);
}

void DropVariableToConditionNodeCommand::Undo()
{
	m_node.m_leftComponent->SetText(m_previousTextBeforeDropping);
}