#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Clock.hpp"
#include <string>

class Renderer;
class Texture;
class PlaySpriteAndDialogueNode;

struct VisualNovelManagerConfig {
	VisualNovelManagerConfig(Renderer& renderer, const AABB2& dialogueBoxUVs = AABB2(Vec2(0.1f, 0.01f), Vec2(0.9f, 0.21f)), const AABB2& nameBoxUVs = AABB2(Vec2(0.02f, 0.245f), Vec2(0.17f, 0.3f)), float textCellHeight = 50.0f, float textCellAspect = 0.75f)
		: m_renderer(renderer), m_dialogueBoxUVs(dialogueBoxUVs), m_nameBoxUVs(nameBoxUVs), m_textCellHeight(textCellHeight), m_textCellAspect(textCellAspect)
	{}
	Renderer& m_renderer;
	AABB2 m_dialogueBoxUVs;
	AABB2 m_nameBoxUVs;
	const float m_textCellHeight;
	const float m_textCellAspect;
};

class VisualNovelManager {
public:
	//screenSpaceAABB does not have to be full screen. i.e. use with popup renderer
	VisualNovelManager(const VisualNovelManagerConfig& config, Clock& parentClock);
	void SetDialogueBoxTexture(Texture* dialogueBoxTexture);
	void SetBackground(Texture* background);
	void SetForeground(Texture* foreground);
	void SetMidground(Texture* midground);

	Texture* GetBackground() const;
	Texture* GetMidground() const;
	Texture* GetForeground() const;

	void SetSpriteUVs(const AABB2& spriteUVs);

	void SetBounds(const AABB2& newBounds);
	AABB2 GetBounds() const;

	void SetDialogueText(const std::string& dialogueText);	//Called from PlaySpriteAndDialogueNode::Tick();
	void SetSpeakerNameText(const std::string& speakerNameText);

	//Update IS Necessary bc it has to know if the player hit the a button or not
	void Update();
	void Render() const;

	Renderer& GetRefToRenderer() const;
	Clock& GetRefToClock();

	unsigned int GetMaxCharsPerDialogueLine() const;

	unsigned int GetMaxNumLines() const;

	void SignalDialogueNodeWaitingForUserInput(PlaySpriteAndDialogueNode& node);

private:
	void UpdateFromKeyboard();
	//void ProcessText();	//JIC you want support for embedding sounds in text, etc

private:
	VisualNovelManagerConfig m_config;
	Clock m_clock;
	AABB2 m_bounds;
	Texture* m_background = nullptr;
	Texture* m_midground = nullptr;
	Texture* m_foreground = nullptr;
	Texture* m_dialogueTexture = nullptr;

	std::string m_fullTextForDialogue;	//The full text received from the node
	std::string m_processedTextForDialogue;	//If the full text is too long it's cut to 3 lines each.

	std::string m_textForSpeakerName;

	AABB2 m_midgroundSpriteUVs;

	std::vector<PlaySpriteAndDialogueNode*> m_historyOfPSDNodes;
	PlaySpriteAndDialogueNode* m_currentWaitingPSDNode = nullptr;

	const unsigned int m_maxNumLines = 3;

	const Vec2 m_triangleUVs = Vec2(0.9f, 0.07f);
};