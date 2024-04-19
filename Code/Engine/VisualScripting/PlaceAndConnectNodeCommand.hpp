#pragma once
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/Math/Vec2.hpp"
#include <string>

class BehaviorTreeEditor;
class BehaviorTreeNode;

class PlaceAndConnectNodeCommand : public Command {
public:
	PlaceAndConnectNodeCommand(BehaviorTreeEditor& editor, const std::string& nodeName, const Vec2& pos, BehaviorTreeNode& nodeToConnectTo, bool shouldConnectToTopPort);
	virtual void Execute() override;
	virtual void Undo() override;

private:
	BehaviorTreeEditor& m_editor;
	const std::string m_nodeName;
	Vec2 m_pos;

	//Fill this up when Execute() runs. Need it for the Undo() command.
	BehaviorTreeNode* m_newlyPlacedNode = nullptr;

	BehaviorTreeNode& m_nodeToConnectTo;
	bool m_shouldConnectToTopPort = false;
};