#include "Engine/VisualNovel/VisualNovelManager.hpp"
#include "Engine/VisualScripting/PlaySpriteAndDialogueNode.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <vector>

VisualNovelManager::VisualNovelManager(const VisualNovelManagerConfig& config, Clock& parentClock): m_config(config), m_clock(parentClock)
{
}

void VisualNovelManager::SetDialogueBoxTexture(Texture* dialogueBoxTexture)
{
	m_dialogueTexture = dialogueBoxTexture;
}

void VisualNovelManager::SetBackground(Texture* background)
{
	m_background = background;
}

void VisualNovelManager::SetForeground(Texture* foreground)
{
	m_foreground = foreground;
}

void VisualNovelManager::SetMidground(Texture* midground)
{
	m_midground = midground;
}

Texture* VisualNovelManager::GetBackground() const
{
	return m_background;
}

Texture* VisualNovelManager::GetMidground() const
{
	return m_midground;
}

Texture* VisualNovelManager::GetForeground() const
{
	return m_foreground;
}

void VisualNovelManager::SetSpriteUVs(const AABB2& spriteUVs)
{
	m_midgroundSpriteUVs = spriteUVs;
}

void VisualNovelManager::SetBounds(const AABB2& newBounds)
{
	m_bounds = newBounds;
}

AABB2 VisualNovelManager::GetBounds() const
{
	return m_bounds;
}

void VisualNovelManager::SetDialogueText(const std::string& dialogueText)
{
	m_fullTextForDialogue = dialogueText;
}

void VisualNovelManager::SetSpeakerNameText(const std::string& speakerNameText)
{
	m_textForSpeakerName = speakerNameText;
}

void VisualNovelManager::Update()
{
	//ProcessText();
	UpdateFromKeyboard();
}

void VisualNovelManager::Render() const
{
	m_config.m_renderer.SetBlendMode(BlendMode::ALPHA);

	std::vector<Vertex_PCU> vertsForBoundsAndPlainUVs;
	AddVertsForAABB2(vertsForBoundsAndPlainUVs, m_bounds, Rgba8::WHITE);
	if (m_background) {
		m_config.m_renderer.BindTexture(m_background);
		m_config.m_renderer.DrawVertexArray((int)vertsForBoundsAndPlainUVs.size(), vertsForBoundsAndPlainUVs.data());
	}

	if (m_midground) {
		std::vector<Vertex_PCU> midgroundVerts;
		AddVertsForAABB2(midgroundVerts, m_bounds, Rgba8::WHITE, m_midgroundSpriteUVs);
		m_config.m_renderer.BindTexture(m_midground);
		m_config.m_renderer.DrawVertexArray((int)midgroundVerts.size(), midgroundVerts.data());
	}

	if (m_foreground) {
		m_config.m_renderer.BindTexture(m_foreground);
		m_config.m_renderer.DrawVertexArray((int)vertsForBoundsAndPlainUVs.size(), vertsForBoundsAndPlainUVs.data());
	}

	if (m_dialogueTexture) {
		m_config.m_renderer.BindTexture(m_dialogueTexture);
		m_config.m_renderer.DrawVertexArray((int)vertsForBoundsAndPlainUVs.size(), vertsForBoundsAndPlainUVs.data());
	}

	std::vector<Vertex_PCU> textVerts;
	BitmapFont* font = m_config.m_renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 dialogueBoxBounds = m_bounds.GetBoxWithin(m_config.m_dialogueBoxUVs.m_mins, m_config.m_dialogueBoxUVs.m_maxs);
	if (m_fullTextForDialogue.size() > 0) {
		font->AddVertsForTextInBox2D(textVerts, dialogueBoxBounds, m_config.m_textCellHeight, m_fullTextForDialogue, Rgba8::WHITE, m_config.m_textCellAspect, Vec2(0.0f, 1.0f), TextBoxMode::OVERRUN);
	}
	if (m_textForSpeakerName.size() > 0) {
		font->AddVertsForTextInBox2D(textVerts, m_bounds.GetBoxWithin(m_config.m_nameBoxUVs.m_mins, m_config.m_nameBoxUVs.m_maxs), m_config.m_textCellHeight, m_textForSpeakerName, Rgba8::WHITE, m_config.m_textCellAspect, Vec2(0.5f, 0.5f), TextBoxMode::OVERRUN);
	}
	m_config.m_renderer.BindTexture(&font->GetTexture());
	m_config.m_renderer.DrawVertexArray((int)textVerts.size(), textVerts.data());

	if (m_currentWaitingPSDNode) {	//If a node is waiting... Let's draw the triangle shape
		std::vector<Vertex_PCU> triangleVerts;
		Vec2 triangleCenter = m_bounds.GetPointAtUV(m_triangleUVs);
		float triangleHalfSize = dialogueBoxBounds.GetShorterDimensionSize() * 0.1f;
		float sqrt3Over2 = Sqrtf(3.0f) / 2.0f;
		triangleVerts.push_back(Vertex_PCU(triangleCenter + Vec2(-sqrt3Over2 * triangleHalfSize, sqrt3Over2 * triangleHalfSize)));
		triangleVerts.push_back(Vertex_PCU(triangleCenter + Vec2(0.0f, -triangleHalfSize)));
		triangleVerts.push_back(Vertex_PCU(triangleCenter + Vec2(sqrt3Over2 * triangleHalfSize, sqrt3Over2 * triangleHalfSize)));

		m_config.m_renderer.BindTexture(nullptr);
		m_config.m_renderer.DrawVertexArray(3, triangleVerts.data());
	}

	/*
	//Debugging
	std::vector<Vertex_PCU> debugVertsForAABB2;
	AABB2 textBoxBounds = m_bounds.GetBoxWithin(m_config.m_dialogueBoxUVs.m_mins, m_config.m_dialogueBoxUVs.m_maxs);
	AddVertsForWireframeAABB2(debugVertsForAABB2, textBoxBounds, 5.0f, Rgba8::WHITE);
	AABB2 nameBoxBounds = m_bounds.GetBoxWithin(m_config.m_nameBoxUVs.m_mins, m_config.m_nameBoxUVs.m_maxs);
	AddVertsForWireframeAABB2(debugVertsForAABB2, nameBoxBounds, 5.0f, Rgba8::WHITE);
	m_config.m_renderer.BindTexture(nullptr);
	m_config.m_renderer.DrawVertexArray((int)debugVertsForAABB2.size(), debugVertsForAABB2.data());
	*/
}

Renderer& VisualNovelManager::GetRefToRenderer() const
{
	return m_config.m_renderer;
}

Clock& VisualNovelManager::GetRefToClock()
{
	return m_clock;
}

unsigned int VisualNovelManager::GetMaxCharsPerDialogueLine() const
{
	GUARANTEE_OR_DIE(!((m_bounds.m_mins == Vec2()) && (m_bounds.m_maxs == Vec2())), "VisualNovelManager bounds not set yet!");
	AABB2 dialogueBox = m_bounds.GetBoxWithin(m_config.m_dialogueBoxUVs.m_mins, m_config.m_dialogueBoxUVs.m_maxs);

	float textCellWidth = m_config.m_textCellHeight* m_config.m_textCellAspect;
	unsigned int maxCharsPerDialogueLine = (unsigned int)(dialogueBox.GetDimensions().x / textCellWidth);
	return maxCharsPerDialogueLine;
}

unsigned int VisualNovelManager::GetMaxNumLines() const
{
	return m_maxNumLines;
}

void VisualNovelManager::UpdateFromKeyboard()
{
	if (m_currentWaitingPSDNode == nullptr)
		return;

	if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE)) {
		m_currentWaitingPSDNode->SignalFromManagerInputPressed();
		m_currentWaitingPSDNode = nullptr;
	}
}

void VisualNovelManager::SignalDialogueNodeWaitingForUserInput(PlaySpriteAndDialogueNode& node)
{
	m_currentWaitingPSDNode = &node;

	bool isNodeIncluded = false;
	for (auto psdNode : m_historyOfPSDNodes) {
		if (psdNode == &node) {
			isNodeIncluded = true;
		}
	}
	
	if (isNodeIncluded == false)
		m_historyOfPSDNodes.push_back(&node);
}