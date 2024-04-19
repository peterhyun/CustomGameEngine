#include "Engine/VisualScripting/PlaySpriteAndDialogueNode.hpp"
#include "Engine/VisualScripting/NodePNGPathOpenComponent.hpp"
#include "Engine/VisualScripting/NodeTextTypeBarComponent.hpp"
#include "Engine/VisualScripting/NodeDropDownComponent.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/VisualNovel/VisualNovelManager.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Input/InputSystem.hpp"

PlaySpriteAndDialogueNode::PlaySpriteAndDialogueNode(unsigned int maxNumCharsPerLineForDialogue, BehaviorTreeEditor* editor, bool overrideDefaultSettings, const Vec2& overridenDim)
	: ActionNode("PlaySpriteAndDialogueNode", overrideDefaultSettings, overridenDim), m_maxNumCharsPerLineForDialogue(maxNumCharsPerLineForDialogue)
{
	m_editor = editor;
	m_spriteNameComponent = new NodePNGPathOpenComponent(*this, Vec2(0.01f, 0.52f), Vec2(0.99f, 0.76f), m_textCellHeight, m_textCellAspect, "Data\\Sprites\\");
	m_spriteDimXComponent = new NodeTextTypeBarComponent(*this, Vec2(0.01f, 0.27f), Vec2(0.09f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::NUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_spriteDimYComponent = new NodeTextTypeBarComponent(*this, Vec2(0.10f, 0.27f), Vec2(0.18f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::NUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_totalSpriteNumComponent = new NodeTextTypeBarComponent(*this, Vec2(0.19f, 0.27f), Vec2(0.35f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::NUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_speakerNameComponent = new NodeTextTypeBarComponent(*this, Vec2(0.36f, 0.27f), Vec2(0.99f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::ALPHABETANDNUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_dialogueTypeComponent = new NodeTextTypeBarComponent(*this, Vec2(0.01f, 0.01f), Vec2(0.99f, 0.26f), m_textCellHeight, m_textCellAspect, false, true, TextType::ANY, BarResizeBehavior::RESIZEBOX, maxNumCharsPerLineForDialogue);

	m_spriteNameComponent->SetDefaultText("SpriteToPlay");
	m_spriteDimXComponent->SetDefaultText("X");
	m_spriteDimYComponent->SetDefaultText("Y");
	m_totalSpriteNumComponent->SetDefaultText("Total");
	m_speakerNameComponent->SetDefaultText("Speaker Name");
	m_dialogueTypeComponent->SetDefaultText("Type Dialogue");

	m_helperComponents.push_back(m_spriteNameComponent);
	m_helperComponents.push_back(m_spriteDimXComponent);
	m_helperComponents.push_back(m_spriteDimYComponent);
	m_helperComponents.push_back(m_totalSpriteNumComponent);
	m_helperComponents.push_back(m_speakerNameComponent);
	m_helperComponents.push_back(m_dialogueTypeComponent);
}

PlaySpriteAndDialogueNode::~PlaySpriteAndDialogueNode()
{	
	delete m_spriteAnimDef;
	delete m_spriteSheet;
	delete m_spriteAndTextClock;
	delete m_textAppearStopwatch;
}

BehaviorTreeTickReturnType PlaySpriteAndDialogueNode::Tick()
{
	GUARANTEE_OR_DIE(m_manager != nullptr, "Have to set the node's m_manager before ticking!");

	if (m_spriteAnimDef == nullptr) {
		GUARANTEE_OR_DIE(m_spriteSheet == nullptr && m_spriteAndTextClock == nullptr && m_textAppearStopwatch == nullptr, "m_spriteSheet, m_clock, and m_textAppearStopwatch should be nullptr when m_spriteAnimDef == nullptr");
		m_spriteAndTextClock = new Clock(m_manager->GetRefToClock());

		std::string xDimStr = m_spriteDimXComponent->GetText();
		std::string yDimStr = m_spriteDimYComponent->GetText();
		std::string spriteName = m_spriteNameComponent->GetText();
		bool didXConvertSucceed;
		int xDim = ConvertStringToInt(xDimStr, didXConvertSucceed);
		bool didYConvertSucceed;
		int yDim = ConvertStringToInt(yDimStr, didYConvertSucceed);
		GUARANTEE_OR_DIE(didXConvertSucceed && didYConvertSucceed, "PlayerSpriteAndDialogueNode::Tick()/ConvertStringToInt on xDim or yDim failed!");

		std::string spriteFullRelativePath = m_spriteNameComponent->GetPathCutoffSubstring() + spriteName + ".png";
		m_spriteSheet = new SpriteSheet(*m_manager->GetRefToRenderer().CreateOrGetTextureFromFile(spriteFullRelativePath.c_str()), IntVec2(xDim, yDim));

		bool didTotalSpriteNumConvertSucceed;
		std::string totalSpriteNumStr = m_totalSpriteNumComponent->GetText();
		int totalSpriteNum = ConvertStringToInt(totalSpriteNumStr, didTotalSpriteNumConvertSucceed);
		GUARANTEE_OR_DIE(didTotalSpriteNumConvertSucceed, "PlayerSpriteAndDialogueNode::Tick()/ConvertStringToInt on totalSpriteNum failed!");
		m_spriteAnimDef = new SpriteAnimDefinition(*m_spriteSheet, 0, totalSpriteNum - 1, m_spriteFPS, SpriteAnimPlaybackType::LOOP);

		const SpriteDefinition spriteDef = m_spriteAnimDef->GetSpriteDefAtTime(0.0f);
		m_manager->SetMidground(&spriteDef.GetTexture());
		m_manager->SetSpriteUVs(spriteDef.GetUVs());

		m_manager->SetSpeakerNameText(m_speakerNameComponent->GetText());

		m_textAppearStopwatch = new Stopwatch(m_spriteAndTextClock, m_textAppearPerLetterTime);
		m_textAppearStopwatch->Start();
		return BehaviorTreeTickReturnType::RUNNING;
	}
	else {
		if (m_didManagerTellMeImDoneFor){
			return BehaviorTreeTickReturnType::SUCCESS;
		}

		float totalSeconds = m_spriteAndTextClock->GetTotalSeconds();

		const SpriteDefinition& spriteDef = m_spriteAnimDef->GetSpriteDefAtTime(totalSeconds);
		m_manager->SetMidground(&spriteDef.GetTexture());
		m_manager->SetSpriteUVs(spriteDef.GetUVs());
		std::string fullDialogue = m_dialogueTypeComponent->GetText();

		while (m_textAppearStopwatch->DecrementDurationIfElapsed()) {
			if ((size_t)m_textSubstrStartIndex + (size_t)m_textSubstrLengthSinceStartIdx >= fullDialogue.size()) {
				m_manager->SignalDialogueNodeWaitingForUserInput(*this);
				break;
			}
			if (fullDialogue.at((size_t)m_textSubstrStartIndex + (size_t)m_textSubstrLengthSinceStartIdx -1) == '\n') {
				m_numberOfLinesToRender++;

				if (m_numberOfLinesToRender % (m_manager->GetMaxNumLines()) == 0) {
					m_textAppearStopwatch->Stop();
					m_manager->SignalDialogueNodeWaitingForUserInput(*this);
					break;
				}
			}
			m_textSubstrLengthSinceStartIdx++;
		}

		m_manager->SetDialogueText(fullDialogue.substr(m_textSubstrStartIndex, m_textSubstrLengthSinceStartIdx));
		return BehaviorTreeTickReturnType::RUNNING;
	}
}

PlaySpriteAndDialogueNode* PlaySpriteAndDialogueNode::Clone(const Vec2& newEditorPos) const
{
	GUARANTEE_OR_DIE(m_childNodes.size() == 0, "There cannot be children for a prototype node");
	PlaySpriteAndDialogueNode* newNode = new PlaySpriteAndDialogueNode(*this);
	newNode->m_posInEditor = newEditorPos;
	return newNode;
}

void PlaySpriteAndDialogueNode::UpdateInEditor()
{
	if (m_isSetupValidWhenLatestValidityCheck == false)
		CheckSetupValidity();

	for (auto helperComponent : m_helperComponents) {
		helperComponent->UpdateInEditor();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_TAB)) {
		if (m_spriteDimXComponent->IsActive()) {
			m_spriteDimXComponent->SetIsActive(false);
			m_spriteDimYComponent->SetIsActive(true);
			return;
		}

		if (m_spriteDimYComponent->IsActive()) {
			m_spriteDimYComponent->SetIsActive(false);
			m_totalSpriteNumComponent->SetIsActive(true);
			return;
		}

		if (m_totalSpriteNumComponent->IsActive()) {
			m_totalSpriteNumComponent->SetIsActive(false);
			m_speakerNameComponent->SetIsActive(true);
			return;
		}

		if (m_speakerNameComponent->IsActive()) {
			m_speakerNameComponent->SetIsActive(false);
			m_dialogueTypeComponent->SetIsActive(true);
			return;
		}

		if (m_dialogueTypeComponent->IsActive()) {
			m_dialogueTypeComponent->SetIsActive(false);
			return;
		}
	}

	UpdateTextVertsForMe();

	std::vector<BehaviorTreeNode*> selectedNodes = m_editor->GetSelectedNodes();
	auto it = std::find(selectedNodes.begin(), selectedNodes.end(), this);
	UpdateOutlineVertsForMe(m_editor->GetCursorPosInEditor(), it != selectedNodes.end());
}

bool PlaySpriteAndDialogueNode::CheckSetupValidity()
{
	std::string totalSpriteNumStr = m_totalSpriteNumComponent->GetText();

	bool convertSpriteNumSuccessful;
	int totalSpriteNum = ConvertStringToInt(totalSpriteNumStr, convertSpriteNumSuccessful);

	std::string xDimStr = m_spriteDimXComponent->GetText();
	std::string yDimStr = m_spriteDimYComponent->GetText();
	bool didXConvertSucceed;
	int xDim = ConvertStringToInt(xDimStr, didXConvertSucceed);
	bool didYConvertSucceed;
	int yDim = ConvertStringToInt(yDimStr, didYConvertSucceed);
	bool isTotalNumberOfSpritesNoMoreThanDimensionProduct = (totalSpriteNum <= xDim * yDim);

	std::string spriteName = m_spriteNameComponent->GetText();

	m_isSetupValidWhenLatestValidityCheck = BehaviorTreeNode::CheckSetupValidity()  && convertSpriteNumSuccessful & didXConvertSucceed && didYConvertSucceed && isTotalNumberOfSpritesNoMoreThanDimensionProduct  && m_spriteNameComponent->GetText().length() != 0 && m_spriteNameComponent->GetPathCutoffSubstring().length() != 0;
	return m_isSetupValidWhenLatestValidityCheck;
}

std::map<std::string, std::string> PlaySpriteAndDialogueNode::GetAndSetXMLAttributes()
{
	BehaviorTreeNode::GetAndSetXMLAttributes();
	m_attributeKeyValues["spriteName"] = m_spriteNameComponent->GetText();
	m_attributeKeyValues["spriteRelativePath"] = m_spriteNameComponent->GetPathCutoffSubstring();
	m_attributeKeyValues["spriteDimX"] = m_spriteDimXComponent->GetText();
	m_attributeKeyValues["spriteDimY"] = m_spriteDimYComponent->GetText();
	m_attributeKeyValues["fullDialogue"] = m_dialogueTypeComponent->GetText();

	m_attributeKeyValues["totalSpriteNum"] = m_totalSpriteNumComponent->GetText();
	m_attributeKeyValues["speakerName"] = m_speakerNameComponent->GetText();

	return m_attributeKeyValues;
}

void PlaySpriteAndDialogueNode::FillInMissingInformationFromXmlElement(const XmlElement& element)
{
	const XmlAttribute* attrib = element.FindAttribute("spriteName");
	GUARANTEE_OR_DIE(attrib != nullptr, "No spriteName attribute!");
	m_spriteNameComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("spriteRelativePath");
	GUARANTEE_OR_DIE(attrib != nullptr, "No spriteRelativePath attribute!");
	m_spriteNameComponent->SetPathCutoffSubstring(attrib->Value());

	attrib = element.FindAttribute("spriteDimX");
	GUARANTEE_OR_DIE(attrib != nullptr, "No spriteDimX attribute!");
	m_spriteDimXComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("spriteDimY");
	GUARANTEE_OR_DIE(attrib != nullptr, "No spriteDimY attribute!");
	m_spriteDimYComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("fullDialogue");
	GUARANTEE_OR_DIE(attrib != nullptr, "No fullDialogue attribute!");
	m_dialogueTypeComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("totalSpriteNum");
	GUARANTEE_OR_DIE(attrib != nullptr, "No totalSpriteNum attribute!");
	m_totalSpriteNumComponent->SetText(attrib->Value());

	attrib = element.FindAttribute("speakerName");
	GUARANTEE_OR_DIE(attrib != nullptr, "No speakerName attribute!");
	m_speakerNameComponent->SetText(attrib->Value());
}

void PlaySpriteAndDialogueNode::UpdateDimensionsRequestedFromComponent(const Vec2& dims, const NodeHelperComponent& component)
{
	if (&component != m_dialogueTypeComponent)
		ERROR_AND_DIE("Only its m_dialogueTypeComponent should request UpdateDimensions for PlaySpriteAndDialogueNode");

	DebuggerPrintf("UpdateDimensionsRequested: %.4f, %.4f\n", dims.x, dims.y);

	m_spriteDimXComponent->RecalculateUVsToPreserveInitialDims(dims);
	m_spriteDimYComponent->RecalculateUVsToPreserveInitialDims(dims);
	m_spriteNameComponent->RecalculateUVsToPreserveInitialDims(dims);
	m_totalSpriteNumComponent->RecalculateUVsToPreserveInitialDims(dims);
	m_speakerNameComponent->RecalculateUVsToPreserveInitialDims(dims);

	m_dimensions = dims;
}

void PlaySpriteAndDialogueNode::AlertTickStopped()
{
	delete m_textAppearStopwatch;
	m_textAppearStopwatch = nullptr;
	delete m_spriteAnimDef;
	m_spriteAnimDef = nullptr;
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
	delete m_spriteAndTextClock;
	m_spriteAndTextClock = nullptr;


	m_numberOfLinesToRender = 0;
	m_didManagerTellMeImDoneFor = false;
	m_textSubstrStartIndex = 0;
	m_textSubstrLengthSinceStartIdx = 1;
}

void PlaySpriteAndDialogueNode::SignalFromManagerInputPressed()
{
	m_numberOfLinesToRender = 0;
	std::string fullDialogue = m_dialogueTypeComponent->GetText();
	m_textSubstrStartIndex += m_textSubstrLengthSinceStartIdx;
	m_textSubstrLengthSinceStartIdx = 1;

	if ((size_t)m_textSubstrStartIndex + (size_t)m_textSubstrLengthSinceStartIdx >= fullDialogue.size()) {
		m_didManagerTellMeImDoneFor = true;
	}
	else {
		m_textAppearStopwatch->Start();
		m_textAppearStopwatch->Restart();
	}
}

PlaySpriteAndDialogueNode::PlaySpriteAndDialogueNode(const PlaySpriteAndDialogueNode& copy) : 
	ActionNode("PlaySpriteAndDialogueNode", true, copy.m_dimensions)
{
	m_editor = copy.m_editor;
	m_manager = copy.m_manager;
	m_spriteNameComponent = new NodePNGPathOpenComponent(*this, Vec2(0.01f, 0.52f), Vec2(0.99f, 0.76f), m_textCellHeight, m_textCellAspect, "Data\\Sprites\\");
	m_spriteDimXComponent = new NodeTextTypeBarComponent(*this, Vec2(0.01f, 0.27f), Vec2(0.09f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::NUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_spriteDimYComponent = new NodeTextTypeBarComponent(*this, Vec2(0.10f, 0.27f), Vec2(0.18f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::NUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_totalSpriteNumComponent = new NodeTextTypeBarComponent(*this, Vec2(0.19f, 0.27f), Vec2(0.35f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::NUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_speakerNameComponent = new NodeTextTypeBarComponent(*this, Vec2(0.36f, 0.27f), Vec2(0.99f, 0.51f), m_textCellHeight, m_textCellAspect, false, false, TextType::ALPHABETANDNUMBERSONLY, BarResizeBehavior::FIXED_LIMITTEXT);
	m_dialogueTypeComponent = new NodeTextTypeBarComponent(*this, Vec2(0.01f, 0.01f), Vec2(0.99f, 0.26f), m_textCellHeight, m_textCellAspect, false, true, TextType::ANY, BarResizeBehavior::RESIZEBOX, copy.m_maxNumCharsPerLineForDialogue);

	m_spriteNameComponent->SetDefaultText("SpriteToPlay");
	m_spriteDimXComponent->SetDefaultText("X");
	m_spriteDimYComponent->SetDefaultText("Y");
	m_totalSpriteNumComponent->SetDefaultText("Total");
	m_speakerNameComponent->SetDefaultText("Speaker Name");
	m_dialogueTypeComponent->SetDefaultText("Type Dialogue");

	m_helperComponents.push_back(m_spriteNameComponent);
	m_helperComponents.push_back(m_spriteDimXComponent);
	m_helperComponents.push_back(m_spriteDimYComponent);
	m_helperComponents.push_back(m_totalSpriteNumComponent);
	m_helperComponents.push_back(m_speakerNameComponent);
	m_helperComponents.push_back(m_dialogueTypeComponent);
}
