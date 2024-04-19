#pragma once
#include "Engine/VisualScripting/ActionNode.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

class NodePNGPathOpenComponent;
class NodeTextTypeBarComponent;
class SpriteAnimDefinition;
class SpriteSheet;
class Clock;
class Stopwatch;

class PlaySpriteAndDialogueNode : public ActionNode {
	friend class DropVariableToPlaySpriteAndDialogueNodeCommand;

public:
	//Might need editor from the constructor cause I need to check if default text fits the bar
	PlaySpriteAndDialogueNode(unsigned int maxNumCharsPerLineForDialogue, BehaviorTreeEditor* editor = nullptr, bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	~PlaySpriteAndDialogueNode();
	BehaviorTreeTickReturnType Tick() override;
	PlaySpriteAndDialogueNode* Clone(const Vec2& newEditorPos) const override;

	//void RegisterVariable(bool isLeftSide, const std::string& variableName);

	virtual void UpdateInEditor() override;
	
	//virtual bool IsPosInTextTypeBar(const Vec2& cursorPosInEditor, bool& isLeftSide) const;

	virtual bool CheckSetupValidity() override;

	virtual std::map<std::string, std::string> GetAndSetXMLAttributes() override;

	virtual void FillInMissingInformationFromXmlElement(const XmlElement& element) override;

	virtual void UpdateDimensionsRequestedFromComponent(const Vec2& dims, const NodeHelperComponent& component) override;

	virtual void AlertTickStopped() override;

	void SignalFromManagerInputPressed();

private:
	PlaySpriteAndDialogueNode(const PlaySpriteAndDialogueNode& copy);

private:

	//You only need this during TICKING. Not while editing
	SpriteAnimDefinition* m_spriteAnimDef = nullptr;
	SpriteSheet* m_spriteSheet = nullptr;
	Clock* m_spriteAndTextClock = nullptr;
	Stopwatch* m_textAppearStopwatch = nullptr;
	unsigned int m_textSubstrStartIndex = 0;
	unsigned int m_textSubstrLengthSinceStartIdx = 1;	//Should always be greater than or equal to 1

	NodePNGPathOpenComponent* m_spriteNameComponent = nullptr;
	NodeTextTypeBarComponent* m_speakerNameComponent = nullptr;
	NodeTextTypeBarComponent* m_spriteDimXComponent = nullptr;
	NodeTextTypeBarComponent* m_spriteDimYComponent = nullptr;
	NodeTextTypeBarComponent* m_totalSpriteNumComponent = nullptr;
	NodeTextTypeBarComponent* m_dialogueTypeComponent = nullptr;

	const float m_textCellHeight = 8.0f;
	const float m_textCellAspect = 0.8f;
	const unsigned int m_maxNumCharsPerLineForDialogue = 0;

	const float m_spriteFPS = 6.0f;
	const float m_textAppearPerLetterTime = 0.08f;

	unsigned int m_numberOfLinesToRender = 0;

	bool m_didManagerTellMeImDoneFor = false;
};