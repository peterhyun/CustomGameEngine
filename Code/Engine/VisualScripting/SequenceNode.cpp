#include "Engine/VisualScripting/SequenceNode.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

SequenceNode::SequenceNode(bool overrideDefaultSettings, const Vec2& overridenDim) : BehaviorTreeNode("SequenceNode", false, overrideDefaultSettings, overridenDim)
{
	m_childNumRange.m_min = 1;
	m_childNumRange.m_max = SEQUENCENODE_MAXCHILDRENNUM;
}

BehaviorTreeTickReturnType SequenceNode::Tick()
{
	int startChildIndex = 0;
	if (m_lastRunningChildIndex != -1) {
		startChildIndex = m_lastRunningChildIndex;
		m_lastRunningChildIndex = -1; // Reset for this tick
	}
	m_lastTickedChild = nullptr;

	int childIndexTracker = 0;
	for (BehaviorTreeNode* childNode : m_childNodes) {
		GUARANTEE_OR_DIE(childNode != nullptr, "childNode should not be a nullptr");
		if (childIndexTracker < startChildIndex) {
			childIndexTracker++;
			continue;
		}
		BehaviorTreeTickReturnType childReturnType = childNode->Tick();
		if (childReturnType == BehaviorTreeTickReturnType::FAILURE) {
			m_lastTickedChild = childNode;
			return BehaviorTreeTickReturnType::FAILURE;
		}
		else if (childReturnType == BehaviorTreeTickReturnType::RUNNING) {
			m_lastRunningChildIndex = childIndexTracker;
			m_lastTickedChild = childNode;
			return BehaviorTreeTickReturnType::RUNNING;
		}
		childIndexTracker++;
	}
	m_lastTickedChild = m_childNodes.back();
	return BehaviorTreeTickReturnType::SUCCESS;
}

SequenceNode* SequenceNode::Clone(const Vec2& newEditorPos) const
{
	GUARANTEE_OR_DIE(m_childNodes.size() == 0, "There cannot be children for a prototype node");
	SequenceNode* newNode = new SequenceNode(*this);
	newNode->m_posInEditor = newEditorPos;
	return newNode;
}

void SequenceNode::AlertTickStopped()
{
	m_lastRunningChildIndex = -1;
}
