#include "Engine/VisualScripting/SetForegroundNode.hpp"
#include "Engine/VisualScripting/NodePNGPathOpenComponent.hpp"
#include "Engine/VisualNovel/VisualNovelManager.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"


SetForegroundNode::SetForegroundNode(BehaviorTreeEditor* editor, bool overrideDefaultSettings, const Vec2& overridenDim) :
	ActionNode("SetForegroundNode", overrideDefaultSettings, overridenDim)
{
	m_editor = editor;
	m_foregroundNameComponent = new NodePNGPathOpenComponent(*this, Vec2(0.01f, 0.1f), Vec2(0.99f, 0.75f), m_textCellHeight, m_textCellAspect, "Data\\Images\\");
	m_foregroundNameComponent->SetDefaultText("ForegroundPNG");

	m_helperComponents.push_back(m_foregroundNameComponent);
}

SetForegroundNode::~SetForegroundNode()
{
}

BehaviorTreeTickReturnType SetForegroundNode::Tick()
{
	GUARANTEE_OR_DIE(m_manager != nullptr, "Have to set the node's m_manager before ticking!");
	std::string pngFullRelativePath = m_foregroundNameComponent->GetPathCutoffSubstring() + m_foregroundNameComponent->GetText() + ".png";
	Texture* foregroundImage = m_manager->GetRefToRenderer().CreateOrGetTextureFromFile(pngFullRelativePath.c_str());
	m_manager->SetForeground(foregroundImage);
	return BehaviorTreeTickReturnType::SUCCESS;
}

SetForegroundNode* SetForegroundNode::Clone(const Vec2& newEditorPos) const
{
	GUARANTEE_OR_DIE(m_childNodes.size() == 0, "There cannot be children for a prototype node");
	SetForegroundNode* newNode = new SetForegroundNode(*this);
	newNode->m_helperComponents.clear();

	newNode->m_foregroundNameComponent = new NodePNGPathOpenComponent(*newNode, Vec2(0.01f, 0.1f), Vec2(0.99f, 0.75f), m_textCellHeight, m_textCellAspect, "Data\\Images\\");
	newNode->m_foregroundNameComponent->SetDefaultText("ForegroundPNG");
	newNode->m_posInEditor = newEditorPos;
	newNode->m_helperComponents.push_back(newNode->m_foregroundNameComponent);
	return newNode;
}

bool SetForegroundNode::CheckSetupValidity()
{
	m_isSetupValidWhenLatestValidityCheck = BehaviorTreeNode::CheckSetupValidity() && (m_foregroundNameComponent->GetPathCutoffSubstring().length() != 0) && (m_foregroundNameComponent->GetText().length() != 0);
	return m_isSetupValidWhenLatestValidityCheck;
}

std::map<std::string, std::string> SetForegroundNode::GetAndSetXMLAttributes()
{
	BehaviorTreeNode::GetAndSetXMLAttributes();
	m_attributeKeyValues["foregroundName"] = m_foregroundNameComponent->GetText();
	m_attributeKeyValues["foregroundRelativePath"] = m_foregroundNameComponent->GetPathCutoffSubstring();
	return m_attributeKeyValues;
}

void SetForegroundNode::FillInMissingInformationFromXmlElement(const XmlElement& element)
{
	const XmlAttribute* attrib = element.FindAttribute("foregroundName");
	GUARANTEE_OR_DIE(attrib != nullptr, "No foregroundName attribute!");
	m_foregroundNameComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("foregroundRelativePath");
	GUARANTEE_OR_DIE(attrib != nullptr, "No foregroundRelativePath attribute!");
	m_foregroundNameComponent->SetPathCutoffSubstring(attrib->Value());
}
