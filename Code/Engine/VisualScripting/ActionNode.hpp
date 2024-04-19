#pragma once
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"

class ActionNode : public BehaviorTreeNode {
public:
	ActionNode(const std::string& actionNodeName, bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	~ActionNode();
	BehaviorTreeTickReturnType Tick() override = 0;
};