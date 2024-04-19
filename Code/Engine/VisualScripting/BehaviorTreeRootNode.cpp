#include "Engine/VisualScripting/BehaviorTreeRootNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/VisualScripting/NodeTextTypeBarComponent.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <queue>

BehaviorTreeRootNode::BehaviorTreeRootNode(bool overrideDefaultSettings, const Vec2& overridenDim) : BehaviorTreeNode("Root Node", true, overrideDefaultSettings, overridenDim)
{
	m_childNumRange.m_max = 1;
	m_isDeletable = false;
	m_isMovable = false;

	m_saveFileNameTypeComponent = new NodeTextTypeBarComponent(*this, Vec2(0.05f, 0.3f), Vec2(0.95f, 0.6f), m_textCellHeight, m_textCellAspect, false, false, TextType::ALPHABETANDNUMBERSONLY, BarResizeBehavior::FIXED_SHRINKTEXT);
	m_saveFileNameTypeComponent->SetDefaultText("XML File Name (no .xml)");

	m_helperComponents.push_back(m_saveFileNameTypeComponent);
}

BehaviorTreeRootNode* BehaviorTreeRootNode::Clone(const Vec2& newPos) const
{
	UNUSED(newPos);
	ERROR_AND_DIE("Clone() should not be called for RootNode");
}

BehaviorTreeTickReturnType BehaviorTreeRootNode::Tick()
{
	if (m_childNodes.size() == 1) {
		m_lastTickedChild = m_childNodes.front();
		return m_childNodes.front()->Tick();
	}
	return BehaviorTreeTickReturnType::SUCCESS;
}

void BehaviorTreeRootNode::SetVNManagerRecursively(VisualNovelManager& manager)
{
	std::queue<BehaviorTreeNode*> nodesToProcess;
	nodesToProcess.push(this);
	while (!nodesToProcess.empty()) {
		BehaviorTreeNode* front = nodesToProcess.front();
		GUARANTEE_OR_DIE(front != nullptr, "Node has nullptr child nodes!");
		front->SetVNManager(manager);
		nodesToProcess.pop();
		for (auto child : front->GetChildNodes()) {
			nodesToProcess.push(child);
		}
	}

}

VisualNovelManager* BehaviorTreeRootNode::GetVNManager() const
{
	return m_manager;
}

bool BehaviorTreeRootNode::CheckSetupValidity()
{
	m_isSetupValidWhenLatestValidityCheck = BehaviorTreeNode::CheckSetupValidity() && (m_saveFileNameTypeComponent->GetText().length() > 0);
	return m_isSetupValidWhenLatestValidityCheck;
}

std::string BehaviorTreeRootNode::GetTypedFileName() const
{
	return m_saveFileNameTypeComponent->GetText();
}

void BehaviorTreeRootNode::SetTypedFileName(const std::string& fileName)
{
	m_saveFileNameTypeComponent->SetText(fileName);
}

std::map<std::string, std::string> BehaviorTreeRootNode::GetAndSetXMLAttributes()
{
	m_attributeKeyValues["fileName"] = m_saveFileNameTypeComponent->GetText();
	return m_attributeKeyValues;
}

void BehaviorTreeRootNode::AlertTickStoppedRecursively()
{
	std::queue<BehaviorTreeNode*> nodesToProcess;
	nodesToProcess.push(this);
	while (!nodesToProcess.empty()) {
		BehaviorTreeNode* front = nodesToProcess.front();
		GUARANTEE_OR_DIE(front != nullptr, "Node has nullptr child nodes!");
		nodesToProcess.pop();
		front->AlertTickStopped();
		for (auto child : front->GetChildNodes()) {
			nodesToProcess.push(child);
		}
	}
}

void BehaviorTreeRootNode::UpdateTickFlowVertsRecursively()
{
	if (m_childNodes.size() == 0)
		return;

	std::vector<BehaviorTreeNode*> arrayOfNodesThatTickedThisUpdate;

	GUARANTEE_OR_DIE(m_lastTickedChild != nullptr, "Root's m_lastTickedChild == nullptr when calling BehaviorTreeRootNode::UpdateTickFlowVertsRecursively()");
	std::queue<BehaviorTreeNode*> nodesToProcess;
	nodesToProcess.push(this);
	while (!nodesToProcess.empty()) {
		BehaviorTreeNode* front = nodesToProcess.front();
		GUARANTEE_OR_DIE(front != nullptr, "nodesToProcess has a nullptr!");
		nodesToProcess.pop();
		arrayOfNodesThatTickedThisUpdate.push_back(front);
		front->UpdateTickFlowIndicatorVertsForMe();

		BehaviorTreeNode* childLastTickedChild = front->GetLastTickedChild();
		if (childLastTickedChild) {
			nodesToProcess.push(childLastTickedChild);
		}
	}

	m_editor->DeleteTickFlowVBOIfNotInThisList(arrayOfNodesThatTickedThisUpdate);
}