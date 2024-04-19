#pragma once
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"

class NodeTextTypeBarComponent;

class BehaviorTreeRootNode : public BehaviorTreeNode {
public:
	BehaviorTreeRootNode(bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	BehaviorTreeRootNode* Clone(const Vec2& newPos) const override;
	BehaviorTreeTickReturnType Tick() override;

	void SetVNManagerRecursively(VisualNovelManager& manager);
	VisualNovelManager* GetVNManager() const;

	virtual bool CheckSetupValidity() override;
	std::string GetTypedFileName() const;
	void SetTypedFileName(const std::string& fileName);

	virtual std::map<std::string, std::string> GetAndSetXMLAttributes() override;

	void AlertTickStoppedRecursively();

	void UpdateTickFlowVertsRecursively();

private:
	NodeTextTypeBarComponent* m_saveFileNameTypeComponent = nullptr;

	float m_textCellHeight = 7.5f;
	float m_textCellAspect = 0.8f;
};