#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/VisualScripting/NodeHelperComponent.hpp"
#include <vector>
#include <string>
#include <functional>

class BehaviorTreeNode;
class Stopwatch;
class NamedProperties;
class BitmapFont;

typedef NamedProperties EventArgs;

enum class TextType {
	INVALID = -1,
	NUMBERSONLY,
	ALPHABETANDNUMBERSONLY,
	ANY
};

enum class BarResizeBehavior {
	INVALID = -1,
	FIXED_SHRINKTEXT,
	FIXED_LIMITTEXT,
	RESIZEBOX
};

class NodeTextTypeBarComponent : public NodeHelperComponent{
public:
	NodeTextTypeBarComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs, float textCellHeight, float textCellAspect, bool isNonTypeable, bool isCtrlEnterNewLine, TextType textType, BarResizeBehavior textBoxResizeType, unsigned int maxNumCharsPerLineWhenResize = 0);
	~NodeTextTypeBarComponent();
	void UpdateInEditor();

	void SetText(const std::string& text);
	void SetDefaultText(const std::string& defaultText);

	bool IsPosInside(const Vec2& cursorPosInEditor);

	std::string GetText() const;

	bool IsActive() const;
	void SetIsActive(bool isActive);

private:
	void UpdateFromKeyboard();
	virtual void UpdateOutlineVertsForMe() override;
	virtual void UpdateTextVertsForMe() override;

	bool DoesTextFitCurrentBox(const std::string& text);
	void ResizeBarIfNecessary(bool shouldShrinkIfPossible);
	
	bool Event_CharPressed(EventArgs& args);
	bool Event_KeyPressed(EventArgs& args);

private:

	Stopwatch* m_caretStopwatch = nullptr;
	bool m_isCaretVisible = false;
	int m_caretPosition = 0;

	std::string m_inputText;
	const float m_textCellHeight = 0.0f;
	const float m_textCellAspect = 0.0f;

	std::string m_defaultText;

	bool m_isNonTypeable = false;
	bool m_isCtrlEnterNewLine = false;
	TextType m_textType = TextType::INVALID;
	BarResizeBehavior m_barResizeBehavior = BarResizeBehavior::INVALID;

	Stopwatch* m_backKeyTimer = nullptr;
	Stopwatch* m_deleteCharAtConstSpeedTimer = nullptr;

	unsigned int m_maxNumCharsPerLine = 0;
};