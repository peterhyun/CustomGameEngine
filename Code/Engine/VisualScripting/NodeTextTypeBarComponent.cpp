#include "Engine/VisualScripting/NodeTextTypeBarComponent.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"

NodeTextTypeBarComponent::NodeTextTypeBarComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs, float textCellHeight, float textCellAspect, bool isNonTypeable, bool isCtrlEnterNewLine, TextType textType, BarResizeBehavior barResizeBehavior, unsigned int maxNumCharsPerLineWhenResize)
	: NodeHelperComponent(ownerNode, nodeUVMins, nodeUVMaxs), m_textCellHeight(textCellHeight), m_textCellAspect(textCellAspect), m_isNonTypeable(isNonTypeable), m_isCtrlEnterNewLine(isCtrlEnterNewLine), m_textType(textType), m_barResizeBehavior(barResizeBehavior), m_maxNumCharsPerLine(maxNumCharsPerLineWhenResize)
{
 	if (m_barResizeBehavior == BarResizeBehavior::FIXED_LIMITTEXT && m_isCtrlEnterNewLine == true) {
		ERROR_AND_DIE("You can't set fixed box limit text while allowing ctrl enter to be a new line!");
	}

	if (m_maxNumCharsPerLine != 0 && m_barResizeBehavior != BarResizeBehavior::RESIZEBOX) {
		ERROR_AND_DIE("You can't specify max number of characters per line if the box resoze behavior isn't RESIZEBOX");
	}

	m_caretStopwatch = new Stopwatch(0.5f);
	m_caretStopwatch->Start();

	m_backKeyTimer = new Stopwatch(0.3f);
	m_backKeyTimer->Stop();

	m_deleteCharAtConstSpeedTimer = new Stopwatch(0.05f);
	m_deleteCharAtConstSpeedTimer->Stop();

	//g_theEventSystem->SubscribeEventCallbackObjectMethod<TextTypeComponent, &TextTypeComponent::Event_KeyPressed>("KeyPressed", *this);
	if (m_isNonTypeable == false) {
		g_theEventSystem->SubscribeEventCallbackObjectMethod<NodeTextTypeBarComponent>("KeyPressed", *this, &NodeTextTypeBarComponent::Event_KeyPressed, 1);
		g_theEventSystem->SubscribeEventCallbackObjectMethod<NodeTextTypeBarComponent>("CharacterPressed", *this, &NodeTextTypeBarComponent::Event_CharPressed, 1);
	}
}

NodeTextTypeBarComponent::~NodeTextTypeBarComponent()
{
	if (m_isNonTypeable == false) {
		g_theEventSystem->UnsubscribeEventCallbackObjectMethod<NodeTextTypeBarComponent>("KeyPressed", *this, &NodeTextTypeBarComponent::Event_KeyPressed);
		g_theEventSystem->UnsubscribeEventCallbackObjectMethod<NodeTextTypeBarComponent>("CharacterPressed", *this, &NodeTextTypeBarComponent::Event_CharPressed);
	}
	delete m_caretStopwatch;
	delete m_backKeyTimer;
	delete m_deleteCharAtConstSpeedTimer;
}

void NodeTextTypeBarComponent::UpdateInEditor()
{
	if (m_isNonTypeable == false) {
		UpdateFromKeyboard();

		if (m_backKeyTimer->HasDurationElapsed() && m_deleteCharAtConstSpeedTimer->IsStopped()) {
			//Activate delete char at const speed timer if you pressed onto the back key long enough.
			m_deleteCharAtConstSpeedTimer->Start();
		}

		if (m_backKeyTimer->IsStopped()) {	//That means the player stopped pressing the back key
			m_deleteCharAtConstSpeedTimer->Restart();
			m_deleteCharAtConstSpeedTimer->Stop();
		}

		if (m_deleteCharAtConstSpeedTimer->DecrementDurationIfElapsed()) {
			if (m_caretPosition > 0) {
				m_inputText.erase(m_caretPosition - 1, 1);
				m_caretPosition--;
				if (m_barResizeBehavior == BarResizeBehavior::RESIZEBOX) {
					ResizeBarIfNecessary(true);
				}
			}
		}

		if (m_isActive == false && m_isCaretVisible == true) {
			m_caretStopwatch->Stop();
			m_isCaretVisible = false;
			return;
		}

		while (m_caretStopwatch->DecrementDurationIfElapsed()) {
			m_isCaretVisible = !m_isCaretVisible;
		}
	}

	UpdateOutlineVertsForMe();
	UpdateTextVertsForMe();
}

void NodeTextTypeBarComponent::SetText(const std::string& text)
{
	if (m_barResizeBehavior == BarResizeBehavior::FIXED_LIMITTEXT) {
		if (DoesTextFitCurrentBox(text) == false) {
			ERROR_AND_DIE("SetText() called for a FIXED_LIMITTEXT bar but too long");
		}
	}

	m_inputText = text;
	m_caretPosition = (int)m_inputText.size();

	if (m_barResizeBehavior == BarResizeBehavior::RESIZEBOX) {
		bool isPreviousTextLonger = m_inputText.size() > text.size();
		if (isPreviousTextLonger) {
			ResizeBarIfNecessary(true);
		}
		else {
			ResizeBarIfNecessary(false);
		}
	}
}

void NodeTextTypeBarComponent::SetDefaultText(const std::string& defaultText)
{
	if (m_barResizeBehavior == BarResizeBehavior::FIXED_LIMITTEXT) {
		if (DoesTextFitCurrentBox(defaultText) == false) {
			ERROR_AND_DIE("SetDefaultText() called for a FIXED_LIMITTEXT bar but too long");
		}
	}

	m_defaultText = defaultText;
}

bool NodeTextTypeBarComponent::IsPosInside(const Vec2& cursorPosInEditor)
{
	AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	return screenAABB2.IsPointInside(cursorPosInEditor);
}

std::string NodeTextTypeBarComponent::GetText() const
{
	return m_inputText;
}

bool NodeTextTypeBarComponent::IsActive() const
{
	return m_isActive;
}

void NodeTextTypeBarComponent::SetIsActive(bool isActive)
{
	m_isActive = isActive;
	if (m_isActive == true) {
		m_caretStopwatch->Start();
	}
}

void NodeTextTypeBarComponent::UpdateFromKeyboard()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
		const BehaviorTreeEditor* editor = m_ownerNode.GetConstPtrToEditor();
		GUARANTEE_OR_DIE(editor != nullptr, "m_ownerNode == nullptr");
		Vec2 pointOnScreen = editor->GetCursorPosInEditor();
		if (IsPointInsideAABB2D(pointOnScreen, screenAABB2)) {
			m_isActive = true;
			m_caretStopwatch->Start();
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
		if (g_theInput->IsKeyDown(KEYCODE_CONTROL) == false) {
			if (m_inputText.size() > 0) {
				m_isActive = false;
			}
		}
		else if (m_isCtrlEnterNewLine) {
			m_inputText.insert(m_caretPosition, 1, '\n');
			m_caretPosition++;
			if (m_barResizeBehavior == BarResizeBehavior::RESIZEBOX) {
				ResizeBarIfNecessary(false);
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW)) {
		if (m_caretPosition > 0) {
			m_caretPosition--;
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW)) {
		if (m_caretPosition < m_inputText.size()) {
			m_caretPosition++;
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_BACKSPACE)) {
		if (m_caretPosition > 0) {
			m_inputText.erase(m_caretPosition - 1, 1);
			m_caretPosition--;
			if (m_backKeyTimer->IsStopped()) {
				m_backKeyTimer->Start();
			}
			if (m_barResizeBehavior == BarResizeBehavior::RESIZEBOX) {
				ResizeBarIfNecessary(true);
			}
		}
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_BACKSPACE)) {
		m_backKeyTimer->Restart();
		m_backKeyTimer->Stop();
	}

	if (g_theInput->IsKeyDown(KEYCODE_CONTROL) && g_theInput->WasKeyJustPressed('V')) {
		std::string stringFromClipboard = Window::PasteTextFromClipboard();
		if (stringFromClipboard.size() > 0) {
			std::string newText = m_inputText;
			newText.insert(m_caretPosition, stringFromClipboard);
			SetText(newText);
		}
	}
}

void NodeTextTypeBarComponent::UpdateOutlineVertsForMe()
{
	m_outlineVerts.clear();

	AABB2 ownerNodeAABB = m_ownerNode.GetAABB2();
	AABB2 componentOutline = ownerNodeAABB.GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	AddVertsForAABB2(m_outlineVerts, componentOutline, Rgba8::WHITE);

	if (m_isCaretVisible == true) {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);

		//Get the single character font width. Also get the adjusted AABB2 if needed
		float finalTextCellHeight = m_textCellHeight;
		AABB2 finalScreenAABB2 = screenAABB2;
		if (m_barResizeBehavior == BarResizeBehavior::FIXED_SHRINKTEXT || m_barResizeBehavior == BarResizeBehavior::RESIZEBOX) {
			//World's messiest code... I hate g_theRenderer not being EngineCommon.hpp
			const BehaviorTreeEditor* editor = m_ownerNode.GetConstPtrToEditor();
			Renderer& renderer = editor->GetRefToRenderer();
			BitmapFont* font = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
			std::vector<Vertex_PCU> dummyVerts;
			if (m_barResizeBehavior == BarResizeBehavior::FIXED_SHRINKTEXT) {
				AddVertsForTextInBoxReturnInfo info = font->AddVertsForTextInBox2D(dummyVerts, screenAABB2, m_textCellHeight, m_inputText, Rgba8::BLACK, m_textCellAspect, Vec2(0.0f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
				finalTextCellHeight = info.m_modifiedCellHeightForShrinkToFit;
			}
			else {
				AddVertsForTextInBoxReturnInfo info = font->AddVertsForTextInBox2D(dummyVerts, screenAABB2, m_textCellHeight, m_inputText, Rgba8::BLACK, m_textCellAspect, Vec2(0.0f, 0.5f), TextBoxMode::OVERRUN, 99999999, true);
				finalScreenAABB2 = info.m_boxThatBoundsAllTextVerts;
			}
		}
		float singleCharacterFontWidth = finalTextCellHeight * m_textCellAspect;

		Vec2 caretStartPosition = Vec2(finalScreenAABB2.m_mins.x + (float)m_caretPosition * singleCharacterFontWidth, finalScreenAABB2.m_mins.y);
		Vec2 caretEndPosition = caretStartPosition + Vec2(0.001f, finalScreenAABB2.GetDimensions().y);
		//TODO: Figure out where caret start position should be if multi-lined
		if (m_isCtrlEnterNewLine) {	//If there is a new line character
			int caretX = 0;
			int caretY = 0;
			int caretOffsetCounter = 0;
			int newLineCharCounter = 0;
			while (caretOffsetCounter < m_caretPosition) {
				if (m_inputText[caretOffsetCounter] != '\n') {
					caretX++;
				}
				else {
					caretX = 0;
					caretY++;
					newLineCharCounter++;
				}
				caretOffsetCounter++;
			}
			Vec2 dims = finalScreenAABB2.GetDimensions();
			float singleLineHeight = dims.y / (newLineCharCounter + 1);
			caretStartPosition = Vec2(finalScreenAABB2.m_mins.x + (float)caretX * singleCharacterFontWidth, finalScreenAABB2.m_mins.y + singleLineHeight * (float)(newLineCharCounter - caretY));
			caretEndPosition = caretStartPosition + Vec2(0.001f, singleLineHeight);
		}
		AddVertsForLineSegment2D(m_outlineVerts, caretStartPosition, caretEndPosition, 2.0f, Rgba8::CHELSEA_GREY);
	}

	Renderer& renderer = m_ownerNode.GetConstPtrToEditor()->GetRefToRenderer();
	if (m_outlineVBO == nullptr) {
		m_outlineVBO = renderer.CreateVertexBuffer(m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "OutlineVBO for NodeTextTypeBarComponent");
	}
	renderer.CopyCPUToGPU(m_outlineVerts.data(), m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_outlineVBO);
}

void NodeTextTypeBarComponent::UpdateTextVertsForMe()
{
	m_textVerts.clear();

	Renderer& renderer = m_ownerNode.GetConstPtrToEditor()->GetRefToRenderer();
	AABB2 buttonScreenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	BitmapFont* bitmapFont = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	GUARANTEE_OR_DIE(bitmapFont != nullptr, "Check BitmapFont file");

	TextBoxMode mode = TextBoxMode::SHRINK_TO_FIT;
	if (m_barResizeBehavior == BarResizeBehavior::RESIZEBOX) {
		mode = TextBoxMode::OVERRUN;
	}

	if (m_inputText.size() > 0) {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
		bitmapFont->AddVertsForTextInBox2D(m_textVerts, screenAABB2, m_textCellHeight, m_inputText, Rgba8::BLACK, m_textCellAspect, Vec2(0.0f, 0.5f), mode, 99999999, true);
	}
	else {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
		bitmapFont->AddVertsForTextInBox2D(m_textVerts, screenAABB2, m_textCellHeight, m_defaultText, Rgba8::PASTEL_BLUE, m_textCellAspect, Vec2(0.0f, 0.5f), mode);
	}

	if (m_textVBO == nullptr) {
		m_textVBO = renderer.CreateVertexBuffer(m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "TextVBO for NodeTextTypeBarComponent");
	}
	renderer.CopyCPUToGPU(m_textVerts.data(), m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
}

bool NodeTextTypeBarComponent::Event_CharPressed(EventArgs& args)
{
	if (m_isActive == false) {
		return false;
	}

	unsigned char charInput = args.GetValue("CharInput", (unsigned char)-1);
	bool didInputTextChange = false;
	if (m_textType == TextType::ALPHABETANDNUMBERSONLY) {
		if ((charInput >= 48 && charInput <= 57) || (charInput >= 65 && charInput <= 90) || (charInput >= 97 && charInput <= 122)) {	//Lower case and upper case alphabets only
			m_inputText.insert(m_caretPosition, 1, charInput);
			m_caretPosition++;
			didInputTextChange = true;
		}
	}
	else if (m_textType == TextType::ANY) {
		if ((charInput >= 32) && (charInput <= 126)) {
			m_inputText.insert(m_caretPosition, 1, charInput);
			m_caretPosition++;
			didInputTextChange = true;
		}
	}
	else if (m_textType == TextType::NUMBERSONLY) {
		if ((charInput >= 48) && (charInput <= 57)) {
			m_inputText.insert(m_caretPosition, 1, charInput);
			m_caretPosition++;
			didInputTextChange = true;
		}
	}
	else {
		ERROR_AND_DIE("Invalid Text Type for NodeTextTypeBarComponent");
	}

	if (didInputTextChange) {
		if (m_barResizeBehavior == BarResizeBehavior::FIXED_LIMITTEXT) {
			if (DoesTextFitCurrentBox(m_inputText) == false) {
				m_inputText.pop_back();
				m_caretPosition--;
			}
		}
		else if (m_barResizeBehavior == BarResizeBehavior::RESIZEBOX) {
			ResizeBarIfNecessary(false);
		}
		return true;
	}
	else {
		return false;
	}
}

bool NodeTextTypeBarComponent::Event_KeyPressed(EventArgs& args)
{
	if (m_isActive == false) {
		return false;
	}

	unsigned char charInput = args.GetValue("KeyCode", (unsigned char)-1);
	if (m_textType == TextType::ALPHABETANDNUMBERSONLY) {
		if ((charInput >= 48 && charInput <= 57) || (charInput >= 65 && charInput <= 90) || (charInput >= 97 && charInput <= 122)) {	//Lower case and upper case alphabets only
			return true;
		}
	}
	else if (m_textType == TextType::ANY){
		if ((charInput >= 32) && (charInput <= 126)) {
			return true;
		}
	}
	else if (m_textType == TextType::NUMBERSONLY) {
		if ((charInput >= 48) && (charInput <= 57)) {
			return true;
		}
	}
	else {
		ERROR_AND_DIE("Invalid Text Type for NodeTextTypeBarComponent");
	}

	return false;
}

bool NodeTextTypeBarComponent::DoesTextFitCurrentBox(const std::string& text)
{
	AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);

	const BehaviorTreeEditor* editor = m_ownerNode.GetConstPtrToEditor();
	GUARANTEE_OR_DIE(editor != nullptr, Stringf("editor is a nullptr for %s", m_ownerNode.GetNodeDisplayStr().c_str()));

	Renderer& renderer = editor->GetRefToRenderer();
	BitmapFont* font = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	std::vector<Vertex_PCU> dummyVerts;
	AddVertsForTextInBoxReturnInfo info = font->AddVertsForTextInBox2D(dummyVerts, screenAABB2, m_textCellHeight, text, Rgba8::BLACK, m_textCellAspect, Vec2(0.0f, 0.5f), TextBoxMode::OVERRUN, 99999999, true);

	return DoesAABB2FitInOtherAABB2(info.m_boxThatBoundsAllTextVerts, screenAABB2);
}

void NodeTextTypeBarComponent::ResizeBarIfNecessary(bool shouldShrinkIfPossible)
{
	//Modify m_inputText based on the maxNumCharsPerLine
	if (m_maxNumCharsPerLine != 0) {
		std::string newInputText;
		unsigned int currentLineNumChars = 0;
		for (std::string::const_iterator it = m_inputText.cbegin();  it != m_inputText.cend(); it++) {
			if (*it != '\n') {
				if (currentLineNumChars < m_maxNumCharsPerLine) {
					newInputText += *it;
					currentLineNumChars++;
				}
				else {
					newInputText += '\n';
					newInputText += *it;
					currentLineNumChars = 0;
					m_caretPosition++;
				}
			}
			else {
				newInputText += '\n';
				currentLineNumChars = 0;
			}
		}
		m_inputText = newInputText;
	}

	AABB2 ownerNodeAABB2 = m_ownerNode.GetAABB2();
	AABB2 screenAABB2 = ownerNodeAABB2.GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	//World's messiest code... I hate g_theRenderer not being EngineCommon.hpp
	const BehaviorTreeEditor* editor = m_ownerNode.GetConstPtrToEditor();
	Renderer& renderer = editor->GetRefToRenderer();
	BitmapFont* font = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	std::vector<Vertex_PCU> dummyVerts;
	AddVertsForTextInBoxReturnInfo info = font->AddVertsForTextInBox2D(dummyVerts, screenAABB2, m_textCellHeight, m_inputText, Rgba8::BLACK, m_textCellAspect, Vec2(0.0f, 0.5f), TextBoxMode::OVERRUN, 99999999, true);

	//Get the initial box sized-aabb2
	AABB2 ownerAABBIfInitialDims = m_ownerNode.GetAABB2ForInitialDims();
	Vec2 componentTopLeft = ownerAABBIfInitialDims.GetTopLeft() + m_initialOffsetFromTopLeft;
	AABB2 initialDimensionsAABB2(Vec2(componentTopLeft.x, componentTopLeft.y - m_initialDimensions.y), Vec2(componentTopLeft.x + m_initialDimensions.x, componentTopLeft.y));
	if (DoesAABB2FitInOtherAABB2(info.m_boxThatBoundsAllTextVerts, initialDimensionsAABB2)) {
		//Don't request node size change. Just recalculate the component UV to match the initial sizes
		GetUVOfOneAABB2RelativeToAnother(initialDimensionsAABB2, ownerNodeAABB2, m_nodeUVMins, m_nodeUVMaxs);
		return;
	}

	//DebuggerPrintf("info.m_boxThatBoundsAllTextVerts: m_mins: %.3f, %.3f / m_maxs: %.3f, %.3f\n", info.m_boxThatBoundsAllTextVerts.m_mins.x, info.m_boxThatBoundsAllTextVerts.m_mins.y, info.m_boxThatBoundsAllTextVerts.m_maxs.x, info.m_boxThatBoundsAllTextVerts.m_maxs.y);
	AABB2 finalMergedAABB2ForNode;
	AABB2 thisComponentMergedAABB2;
	if (shouldShrinkIfPossible) {
		thisComponentMergedAABB2 = MergeAABB2s(info.m_boxThatBoundsAllTextVerts, initialDimensionsAABB2);
		finalMergedAABB2ForNode = MergeAABB2s(thisComponentMergedAABB2, ownerAABBIfInitialDims);
	}
	else {
		thisComponentMergedAABB2 = MergeAABB2s(info.m_boxThatBoundsAllTextVerts, initialDimensionsAABB2);
		finalMergedAABB2ForNode = MergeAABB2s(thisComponentMergedAABB2, ownerNodeAABB2);
	}

	//DebuggerPrintf("finalMergedAABB2ForNode: m_mins: %.3f, %.3f / m_maxs: %.3f, %.3f\n", finalMergedAABB2ForNode.m_mins.x, finalMergedAABB2ForNode.m_mins.y, finalMergedAABB2ForNode.m_maxs.x, finalMergedAABB2ForNode.m_maxs.y);

	//Updates the uvs of other components and the size of the owner node
	m_ownerNode.UpdateDimensionsRequestedFromComponent(finalMergedAABB2ForNode.GetDimensions(), *this);

	//Then change the UVs of this component
	GetUVOfOneAABB2RelativeToAnother(thisComponentMergedAABB2, finalMergedAABB2ForNode, m_nodeUVMins, m_nodeUVMaxs);
}
