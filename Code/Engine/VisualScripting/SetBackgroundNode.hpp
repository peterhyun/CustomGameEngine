#pragma once
#include "Engine/VisualScripting/ActionNode.hpp"

class NodePNGPathOpenComponent;
class BehaviorTreeEditor;

class SetBackgroundNode : public ActionNode {
public:
	SetBackgroundNode(BehaviorTreeEditor* editor = nullptr, bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	~SetBackgroundNode();
	BehaviorTreeTickReturnType Tick() override;
	SetBackgroundNode* Clone(const Vec2& newEditorPos) const override;

	//void RegisterVariable(bool isLeftSide, const std::string& variableName);

	virtual bool CheckSetupValidity() override;

	virtual std::map<std::string, std::string> GetAndSetXMLAttributes() override;

	virtual void FillInMissingInformationFromXmlElement(const XmlElement& element) override;

private:
	NodePNGPathOpenComponent* m_backgroundNameComponent = nullptr;

	const float m_textCellHeight = 8.0f;
	const float m_textCellAspect = 0.8f;
};