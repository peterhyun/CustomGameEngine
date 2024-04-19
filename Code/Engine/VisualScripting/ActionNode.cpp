#include "Engine/VisualScripting/ActionNode.hpp"

ActionNode::ActionNode(const std::string& actionNodeName, bool overrideDefaultSettings, const Vec2& overridenDim) : BehaviorTreeNode(actionNodeName, false, overrideDefaultSettings, overridenDim)
{
	m_childNumRange.m_min = 0;
	m_childNumRange.m_max = 0;
}

ActionNode::~ActionNode()
{
}
