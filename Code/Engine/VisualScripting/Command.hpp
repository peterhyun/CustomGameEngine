#pragma once

//This is a pure class! Please inherit from it.
class Command {
public:
	Command() {};
	virtual ~Command() {};
	virtual void Execute() = 0;
	virtual void Undo() = 0;
};