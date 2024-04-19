#pragma once
#include "Engine/VisualScripting/ActionNode.hpp"

class NodePNGPathOpenComponent;
class BehaviorTreeEditor;

class SetForegroundNode : public ActionNode {
public:
	SetForegroundNode(BehaviorTreeEditor* editor = nullptr, bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	~SetForegroundNode();
	BehaviorTreeTickReturnType Tick() override;
	SetForegroundNode* Clone(const Vec2& newEditorPos) const override;

	//void RegisterVariable(bool isLeftSide, const std::string& variableName);

	virtual bool CheckSetupValidity() override;

	virtual std::map<std::string, std::string> GetAndSetXMLAttributes() override;

	virtual void FillInMissingInformationFromXmlElement(const XmlElement& element) override;

private:
	NodePNGPathOpenComponent* m_foregroundNameComponent = nullptr;

	const float m_textCellHeight = 8.0f;
	const float m_textCellAspect = 0.8f;
};