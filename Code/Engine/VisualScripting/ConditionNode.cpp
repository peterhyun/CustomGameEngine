#include "Engine/VisualScripting/ConditionNode.hpp"
#include "Engine/VisualScripting/NodeDropDownComponent.hpp"
#include "Engine/VisualScripting/NodeTextTypeBarComponent.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"

ConditionNode::ConditionNode(BehaviorTreeEditor* editor, bool overrideDefaultSettings, const Vec2& overridenDim) : BehaviorTreeNode("ConditionNode", false, overrideDefaultSettings, overridenDim)
{	
	m_editor = editor;

	std::vector<std::string> operatorStrings = {">", "<", ">=", "<=", "=="};
	m_dropDownComponent = new NodeDropDownComponent(*this, Vec2(0.1f, 0.3f), Vec2(0.9f, 0.7f), operatorStrings, Vec2(TREENODE_DIMX * 0.5f, TREENODE_DIMY * 0.2f));
	m_leftComponent = new NodeTextTypeBarComponent(*this, Vec2(0.01f, 0.1f), Vec2(0.46f, 0.25f), m_textCellHeight, m_textCellAspect, true, false, TextType::INVALID, BarResizeBehavior::RESIZEBOX);
	m_rightComponent = new NodeTextTypeBarComponent(*this, Vec2(0.54f, 0.1f), Vec2(0.99f, 0.25f), m_textCellHeight, m_textCellAspect, false, false, TextType::ALPHABETANDNUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);

	m_leftComponent->SetDefaultText("Variable");
	m_rightComponent->SetDefaultText("Literal");

	m_helperComponents.push_back(m_dropDownComponent);
	m_helperComponents.push_back(m_leftComponent);
	m_helperComponents.push_back(m_rightComponent);

	//This is a leaf node so NO children!
	m_childNumRange.m_min = 0;
	m_childNumRange.m_max = 0;
}

ConditionNode::~ConditionNode()
{
}

BehaviorTreeTickReturnType ConditionNode::Tick()	//Should only return either success or failure
{
	NamedProperties keyVals = m_editor->GetBlackboardValues();
	std::string leftText = m_leftComponent->GetText();
	GUARANTEE_OR_DIE(keyVals.HasKey(leftText), Stringf("Blackboard doesn't have the key: %s", leftText.c_str()));

	auto pairs = keyVals.m_keyValuePairs;
	NamedPropertyBase* lhsPropertyBase = pairs.find(leftText)->second;
	NamedPropertyOfType<bool>* boolProp = dynamic_cast<NamedPropertyOfType<bool>*>(lhsPropertyBase);
	NamedPropertyOfType<int>* intProp = dynamic_cast<NamedPropertyOfType<int>*>(lhsPropertyBase);
	if (boolProp) {
		bool leftBoolVal = boolProp->GetValue();

		std::string rightText = m_rightComponent->GetText();
		bool rightBoolVal = false;
		if (rightText == "true" || rightText == "True") {
			rightBoolVal = true;
		}
		else if (rightText == "false" || rightText == "False") {
			rightBoolVal = false;
		}
		else {
			//Check if right text is also a variable
			NamedPropertyBase* rhsPropertyBase = pairs.find(rightText)->second;
			NamedPropertyOfType<bool>* rhsBoolProp = dynamic_cast<NamedPropertyOfType<bool>*>(rhsPropertyBase);
			GUARANTEE_OR_DIE(rhsBoolProp != nullptr, "rhsBoolProp == nullptr");
			rightBoolVal = rhsBoolProp->GetValue();
		}
		if (DoCompareOperation(leftBoolVal, rightBoolVal)) {
			return BehaviorTreeTickReturnType::SUCCESS;
		}
		else {
			return BehaviorTreeTickReturnType::FAILURE;
		}
	}
	else if (intProp) {
		int leftIntVal = intProp->GetValue();

		std::string rightText = m_rightComponent->GetText();
		int rightIntVal = 0;

		bool wasConvertSuccessful;
		rightIntVal	= ConvertStringToInt(rightText, wasConvertSuccessful);

		if (wasConvertSuccessful == false) {
			//Check if right text is also a variable
			NamedPropertyBase* rhsPropertyBase = pairs.find(rightText)->second;
			NamedPropertyOfType<int>* rhsIntProp = dynamic_cast<NamedPropertyOfType<int>*>(rhsPropertyBase);
			GUARANTEE_OR_DIE(rhsIntProp != nullptr, "rhsIntProp == nullptr");
			rightIntVal = rhsIntProp->GetValue();
		}

		if (DoCompareOperation(leftIntVal, rightIntVal)) {
			return BehaviorTreeTickReturnType::SUCCESS;
		}
		else {
			return BehaviorTreeTickReturnType::FAILURE;
		}
	}
	else {
		ERROR_AND_DIE("Variable should be either bool or int");
	}
}

ConditionNode* ConditionNode::Clone(const Vec2& newEditorPos) const
{
	GUARANTEE_OR_DIE(m_childNodes.size() == 0, "There cannot be children for a prototype node");
	ConditionNode* newNode = new ConditionNode(*this);
	newNode->m_helperComponents.clear();

	newNode->m_posInEditor = newEditorPos;
	std::vector<std::string> operatorStrings = { ">", "<", ">=", "<=", "==" };
	newNode->m_dropDownComponent = new NodeDropDownComponent(*newNode, Vec2(0.1f, 0.3f), Vec2(0.9f, 0.7f), operatorStrings, Vec2(TREENODE_DIMX * 0.5f, TREENODE_DIMY * 0.2f));
	newNode->m_leftComponent = new NodeTextTypeBarComponent(*newNode, Vec2(0.01f, 0.1f), Vec2(0.46f, 0.25f), m_textCellHeight, m_textCellAspect, true, false, TextType::INVALID, BarResizeBehavior::RESIZEBOX);
	newNode->m_rightComponent = new NodeTextTypeBarComponent(*newNode, Vec2(0.54f, 0.1f), Vec2(0.99f, 0.25f), m_textCellHeight, m_textCellAspect, false, false, TextType::ALPHABETANDNUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);

	newNode->m_leftComponent->SetDefaultText("Variable");
	newNode->m_rightComponent->SetDefaultText("Literal");

	newNode->m_helperComponents.push_back(newNode->m_dropDownComponent);
	newNode->m_helperComponents.push_back(newNode->m_leftComponent);
	newNode->m_helperComponents.push_back(newNode->m_rightComponent);

	return newNode;
}

/*
void ConditionNode::RegisterVariable(bool isLeftSide, const std::string& variableName)
{
	if (isLeftSide) {
		m_leftComponent->SetText(variableName);
	}
	else {
		m_rightComponent->SetText(variableName);
	}
}
*/

void ConditionNode::UpdateInEditor()
{
	if (m_isSetupValidWhenLatestValidityCheck == false)
		CheckSetupValidity();

	for (auto helperComponent : m_helperComponents) {
		helperComponent->UpdateInEditor();
	}

	int selectedOptionIdx = m_dropDownComponent->GetSelectedOptionIdx();
	if (selectedOptionIdx != -1) {
		m_currentOperator = (ConditionNodeOperator)selectedOptionIdx;
	}

	UpdateTextVertsForMe();

	std::vector<BehaviorTreeNode*> selectedNodes = m_editor->GetSelectedNodes();
	auto it = std::find(selectedNodes.begin(), selectedNodes.end(), this);
	UpdateOutlineVertsForMe(m_editor->GetCursorPosInEditor(), it != selectedNodes.end());
}

bool ConditionNode::IsPosInTextTypeBar(const Vec2& cursorPosInEditor, bool& isLeftSide) const
{
	if (m_leftComponent->IsPosInside(cursorPosInEditor)) {
		isLeftSide = true;
		return true;
	}
	else  if (m_rightComponent->IsPosInside(cursorPosInEditor)){
		isLeftSide = false;
		return true;
	}
	return false;
}

bool ConditionNode::CheckSetupValidity()
{
	NamedProperties keyVals = m_editor->GetBlackboardValues();

	bool isLeftHandSideKeyVal = true;
	if (keyVals.HasKey(m_leftComponent->GetText()) == false) {
		isLeftHandSideKeyVal = false;
	}

	bool isRightHandSideKeyVal = true;
	if (keyVals.HasKey(m_rightComponent->GetText()) == false) {
		isRightHandSideKeyVal = false;
	}

	std::string rightText = m_rightComponent->GetText(); 
	bool wasConvertSuccessful = false;
	ConvertStringToInt(rightText, wasConvertSuccessful);

	bool isRightHandSideKeyValOrLiteral = isRightHandSideKeyVal || (rightText == "true") || (rightText == "false") || (rightText == "True") || (rightText == "False") || wasConvertSuccessful;

	m_isSetupValidWhenLatestValidityCheck = (BehaviorTreeNode::CheckSetupValidity() && m_currentOperator != ConditionNodeOperator::INVALID && isLeftHandSideKeyVal && isRightHandSideKeyValOrLiteral);
	return m_isSetupValidWhenLatestValidityCheck;
}

std::map<std::string, std::string> ConditionNode::GetAndSetXMLAttributes()
{
	BehaviorTreeNode::GetAndSetXMLAttributes();
	m_attributeKeyValues["lhsStr"] = m_leftComponent->GetText();
	m_attributeKeyValues["rhsStr"] = m_rightComponent->GetText();
	m_attributeKeyValues["operator"] = std::to_string((int)m_currentOperator);
	return m_attributeKeyValues;
}

void ConditionNode::FillInMissingInformationFromXmlElement(const XmlElement& element)
{
	const XmlAttribute* attrib = element.FindAttribute("lhsStr");
	GUARANTEE_OR_DIE(attrib != nullptr, "No lhsStr attribute!");
	m_leftComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("rhsStr");
	GUARANTEE_OR_DIE(attrib != nullptr, "No rhsStr attribute!");
	m_rightComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("operator");
	GUARANTEE_OR_DIE(attrib != nullptr, "No operator attribute!");
	
	bool wasConvertSuccessful = false;
	int comparisonEnumInt = ConvertStringToInt(attrib->Value(), wasConvertSuccessful);
	m_dropDownComponent->SetSelectedOptionIdx((int)comparisonEnumInt);
	m_currentOperator = (ConditionNodeOperator)comparisonEnumInt;
}

/*
void ConditionNode::UpdateOutlineVertsForMe(const Vec2& cursorPosInEditor, bool isHighlighted)
{
	BehaviorTreeNode::UpdateOutlineVertsForMe(cursorPosInEditor, isHighlighted);
	m_leftComponent->UpdateOutlineVertsForMe();
	m_rightComponent->UpdateOutlineVertsForMe();
	m_dropDownComponent->UpdateOutlineVertsForMe();
}

void ConditionNode::UpdateTextVertsForMe()
{
	BehaviorTreeNode::UpdateTextVertsForMe();
	m_leftComponent->UpdateTextVertsForMe(bitmapFont, out_verts);
	m_rightComponent->UpdateTextVertsForMe(bitmapFont, out_verts);
	m_dropDownComponent->UpdateTextVertsForMe(bitmapFont, out_verts);
}
*/