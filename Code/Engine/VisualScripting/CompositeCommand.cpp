#include "Engine/VisualScripting/CompositeCommand.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

CompositeCommand::CompositeCommand(const std::vector<Command*>& commands) : m_commands(commands)
{
}

CompositeCommand::~CompositeCommand()
{
	for (Command* command : m_commands) {
		delete command;
	}
}

void CompositeCommand::Execute()
{
	for (Command* command : m_commands) {
		if (command) {
			command->Execute();
		}
		else {
			ERROR_AND_DIE("CompositeCommand::m_commands cannot contain a nullptr!");
		}
	}
}

void CompositeCommand::Undo()
{
	for (Command* command : m_commands) {
		if (command) {
			command->Undo();
		}
		else {
			ERROR_AND_DIE("CompositeCommand::m_commands cannot contain a nullptr!");
		}
	}
}
