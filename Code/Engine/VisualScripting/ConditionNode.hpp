#pragma once
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

class NodeDropDownComponent;
class NodeTextTypeBarComponent;
class BehaviorTreeEditor;

class ConditionNode : public BehaviorTreeNode {
	friend class DropVariableToConditionNodeCommand;
public:
	ConditionNode(BehaviorTreeEditor* editor = nullptr, bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	~ConditionNode();
	BehaviorTreeTickReturnType Tick() override;
	ConditionNode* Clone(const Vec2& newEditorPos) const override;

	//void RegisterVariable(bool isLeftSide, const std::string& variableName);

	virtual void UpdateInEditor() override;

	virtual bool IsPosInTextTypeBar(const Vec2& cursorPosInEditor, bool& isLeftSide) const;

	virtual bool CheckSetupValidity() override;

	virtual std::map<std::string, std::string> GetAndSetXMLAttributes() override;

	virtual void FillInMissingInformationFromXmlElement(const XmlElement& element) override;

private:
	template<typename T>
	bool DoCompareOperation(T lhsVal, T rhsVal) const;

private:
	NodeDropDownComponent* m_dropDownComponent = nullptr;
	NodeTextTypeBarComponent* m_leftComponent = nullptr;
	NodeTextTypeBarComponent* m_rightComponent = nullptr;

	enum class ConditionNodeOperator {
		INVALID = -1,
		GREATERTHAN,
		LESSTHAN,
		GREATERTHANOREQUAL,
		LESSTHANOREQUAL,
		EQUAL
	};
	ConditionNodeOperator m_currentOperator = ConditionNodeOperator::INVALID;

	const float m_textCellHeight = 7.5f;
	const float m_textCellAspect = 0.8f;
};

template<typename T>
inline bool ConditionNode::DoCompareOperation(T lhsVal, T rhsVal) const
{
	switch (m_currentOperator) {
	case ConditionNodeOperator::GREATERTHAN :
		return lhsVal > rhsVal;
	case ConditionNodeOperator::LESSTHAN :
		return lhsVal < rhsVal;
	case ConditionNodeOperator::GREATERTHANOREQUAL :
		return lhsVal >= rhsVal;
	case ConditionNodeOperator::LESSTHANOREQUAL :
		return lhsVal <= rhsVal;
	case ConditionNodeOperator::EQUAL :
		return lhsVal == rhsVal;
	default:
		ERROR_AND_DIE("INVALID COMPARE OPERATOR");
	}
}