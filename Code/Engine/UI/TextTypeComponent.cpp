#include "Engine/UI/TextTypeComponent.hpp"
#include "Engine/UI/Widget.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"

TextTypeComponent::TextTypeComponent(Renderer& renderer, const Widget& ownerWidget, const AABB2& screenAABB2, float textCellHeight) : m_renderer(renderer), m_ownerWidget(ownerWidget), m_screenAABB2(screenAABB2), m_textCellHeight(textCellHeight)
{
	m_bitmapFont = m_renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	m_caretStopwatch = new Stopwatch(0.5f);
	m_caretStopwatch->Start();

	//g_theEventSystem->SubscribeEventCallbackObjectMethod<TextTypeComponent, &TextTypeComponent::Event_KeyPressed>("KeyPressed", *this);
	g_theEventSystem->SubscribeEventCallbackObjectMethod<TextTypeComponent>("CharacterPressed", *this, &TextTypeComponent::Event_CharPressed);
}

TextTypeComponent::~TextTypeComponent()
{
	delete m_caretStopwatch;
	m_caretStopwatch = nullptr;
}

void TextTypeComponent::Update()
{
	UpdateFromKeyboard();

	if (m_isActive == false) {
		m_isCaretVisible = false;
		return;
	}

	while (m_caretStopwatch->DecrementDurationIfElapsed()) {
		m_isCaretVisible = !m_isCaretVisible;
	}
}

void TextTypeComponent::Render() const
{
	std::vector<Vertex_PCU> textVerts;
	textVerts.reserve(128);

	float cellAspect = 0.8f;
	AddVertsForTextInBoxReturnInfo info  = m_bitmapFont->AddVertsForTextInBox2D(textVerts, m_screenAABB2, m_textCellHeight, m_inputText, Rgba8::BLACK, cellAspect, Vec2(0.0f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
	float newTextCellHeight = info.m_modifiedCellHeightForShrinkToFit;

	std::vector<Vertex_PCU> nonTextVerts;
	nonTextVerts.reserve(128);

	AddVertsForAABB2(nonTextVerts, m_screenAABB2, Rgba8::LIGHTGREEN);
	if (m_isCaretVisible == true) {
		float singleCharacterFontWidth = newTextCellHeight * cellAspect;
		Vec2 caretStartPosition = Vec2(m_screenAABB2.m_mins.x + (float)m_caretPosition * singleCharacterFontWidth, m_screenAABB2.m_mins.y);
		Vec2 caretEndPosition = caretStartPosition + Vec2(0.001f, m_screenAABB2.GetDimensions().y);
		//Render the carets
		AddVertsForLineSegment2D(nonTextVerts, caretStartPosition, caretEndPosition, 4.0f, Rgba8::PURPLE);
	}

	m_renderer.BindTexture(nullptr);
	m_renderer.SetModelConstants();
	m_renderer.DrawVertexArray((int)nonTextVerts.size(), nonTextVerts.data());

	if (textVerts.size() > 0) {
		m_renderer.BindTexture(&m_bitmapFont->GetTexture());
		m_renderer.DrawVertexArray((int)textVerts.size(), textVerts.data());
	}
}

void TextTypeComponent::RegisterEnterKeyCallbackFunction(std::function<bool(std::string)> function)
{
	m_enterKeyCallbackFunction = function;
}

void TextTypeComponent::UpdateFromKeyboard()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 pointOnScreen = m_ownerWidget.GetConstRefToOverlay().GetBounds().GetPointAtUV(normalizedCursorPos);
		if (IsPointInsideAABB2D(pointOnScreen, m_screenAABB2)) {
			m_isActive = true;
		}
		else {
			m_isActive = false;
		}
	}

	if (m_isActive == false) {
		return;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC)) {
		m_isActive = false;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER)) {
		if (m_inputText.size() > 0) {
			bool returnResult = false;
			if (m_enterKeyCallbackFunction) {
				returnResult = m_enterKeyCallbackFunction(m_inputText);
			}
			if (returnResult == true) {
				m_inputText.clear();
				m_caretPosition = 0;
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW)) {
		if (m_caretPosition > 0) {
			m_caretPosition--;
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW)) {
		if (m_caretPosition < m_inputText.size()) {
			m_caretPosition++;
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEYCODE_BACKSPACE)) {
		if (m_caretPosition > 0) {
			m_inputText.erase(m_caretPosition - 1, 1);
			m_caretPosition--;
		}
	}
}

bool TextTypeComponent::Event_CharPressed(EventArgs& args)
{
	if (m_isActive == false) {
		return false;
	}

	unsigned char charInput = args.GetValue("CharInput", (unsigned char)-1);
	if ((charInput >= 65 && charInput <= 90) || (charInput >= 97 && charInput <= 122)) {	//Lower case and upper case alphabets only
		m_inputText.insert(m_caretPosition, 1, charInput);
		m_caretPosition++;
	}

	return true;
}
