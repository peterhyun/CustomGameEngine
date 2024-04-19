#pragma once
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"

constexpr int SEQUENCENODE_MAXCHILDRENNUM = 10;

class SequenceNode : public BehaviorTreeNode {
public:
	SequenceNode(bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	BehaviorTreeTickReturnType Tick() override;
	SequenceNode* Clone(const Vec2& newEditorPos) const override;

	virtual void AlertTickStopped() override;

private:
	int m_lastRunningChildIndex = -1; // -1 indicates no child was running
};