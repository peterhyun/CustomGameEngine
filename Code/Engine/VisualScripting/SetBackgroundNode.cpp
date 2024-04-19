#include "Engine/VisualScripting/SetBackgroundNode.hpp"
#include "Engine/VisualScripting/NodePNGPathOpenComponent.hpp"
#include "Engine/VisualNovel/VisualNovelManager.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"


SetBackgroundNode::SetBackgroundNode(BehaviorTreeEditor* editor, bool overrideDefaultSettings, const Vec2& overridenDim) :
	ActionNode("SetBackgroundNode", overrideDefaultSettings, overridenDim)
{
	m_editor = editor;
	m_backgroundNameComponent = new NodePNGPathOpenComponent(*this, Vec2(0.01f, 0.1f), Vec2(0.9f, 0.75f), m_textCellHeight, m_textCellAspect, "Data\\Images\\");
	m_backgroundNameComponent->SetDefaultText("BackgroundPNG");

	m_helperComponents.push_back(m_backgroundNameComponent);
}

SetBackgroundNode::~SetBackgroundNode()
{
}

BehaviorTreeTickReturnType SetBackgroundNode::Tick()
{
	GUARANTEE_OR_DIE(m_manager != nullptr, "Have to set the node's m_manager before ticking!");
	std::string pngFullRelativePath = m_backgroundNameComponent->GetPathCutoffSubstring() + m_backgroundNameComponent->GetText() + ".png";
	Texture* backgroundImage = m_manager->GetRefToRenderer().CreateOrGetTextureFromFile(pngFullRelativePath.c_str());
	m_manager->SetBackground(backgroundImage);
	return BehaviorTreeTickReturnType::SUCCESS;
}

SetBackgroundNode* SetBackgroundNode::Clone(const Vec2& newEditorPos) const
{
	GUARANTEE_OR_DIE(m_childNodes.size() == 0, "There cannot be children for a prototype node");
	SetBackgroundNode* newNode = new SetBackgroundNode(*this);
	newNode->m_helperComponents.clear();

	newNode->m_backgroundNameComponent = new NodePNGPathOpenComponent(*newNode, Vec2(0.01f, 0.1f), Vec2(0.99f, 0.75f), m_textCellHeight, m_textCellAspect, "Data\\Images\\");
	newNode->m_backgroundNameComponent->SetDefaultText("BackgroundPNG");
	newNode->m_posInEditor = newEditorPos;

	newNode->m_helperComponents.push_back(newNode->m_backgroundNameComponent);

	return newNode;
}

bool SetBackgroundNode::CheckSetupValidity()
{
	m_isSetupValidWhenLatestValidityCheck = BehaviorTreeNode::CheckSetupValidity() && (m_backgroundNameComponent->GetPathCutoffSubstring().length() != 0) && (m_backgroundNameComponent->GetText().length() != 0);
	return m_isSetupValidWhenLatestValidityCheck;
}

std::map<std::string, std::string> SetBackgroundNode::GetAndSetXMLAttributes()
{
	BehaviorTreeNode::GetAndSetXMLAttributes();
	m_attributeKeyValues["backgroundName"] = m_backgroundNameComponent->GetText();
	m_attributeKeyValues["backgroundRelativePath"] = m_backgroundNameComponent->GetPathCutoffSubstring();
	return m_attributeKeyValues;
}

void SetBackgroundNode::FillInMissingInformationFromXmlElement(const XmlElement& element)
{
	const XmlAttribute* attrib = element.FindAttribute("backgroundName");
	GUARANTEE_OR_DIE(attrib != nullptr, "No backgroundName attribute!");
	m_backgroundNameComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("backgroundRelativePath");
	GUARANTEE_OR_DIE(attrib != nullptr, "No backgroundRelativePath attribute!");
	m_backgroundNameComponent->SetPathCutoffSubstring(attrib->Value());
}
