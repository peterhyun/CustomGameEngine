#pragma once
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"

constexpr int FALLBACKNODE_MAXCHILDRENNUM = 10;

class FallbackNode : public BehaviorTreeNode {
public:
	FallbackNode(bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	BehaviorTreeTickReturnType Tick() override;
	FallbackNode* Clone(const Vec2& newEditorPos) const override;

	virtual void AlertTickStopped() override;

private:
	int m_lastRunningChildIndex = -1; // -1 indicates no child was running
};