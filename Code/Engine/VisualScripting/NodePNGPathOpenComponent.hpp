#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/VisualScripting/NodeHelperComponent.hpp"
#include <vector>
#include <string>
#include <functional>

class BehaviorTreeNode;
class NamedProperties;
class BitmapFont;

typedef NamedProperties EventArgs;

constexpr float CARETSTOPWATCHTIME = 0.5f;

class NodePNGPathOpenComponent : public NodeHelperComponent {
public:
	NodePNGPathOpenComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs, float textCellHeight, float textCellAspect, const std::string& pathCutoffSubstring);
	~NodePNGPathOpenComponent();
	void UpdateInEditor();

	void SetText(const std::string& text);
	void SetDefaultText(const std::string& defaultText);

	bool IsPosInside(const Vec2& cursorPosInEditor);

	std::string GetText() const;

	std::string GetPathCutoffSubstring() const;

	void SetPathCutoffSubstring(const std::string& pathCutoffSubstring);

private:
	void UpdateFromKeyboard();	//<- TODO: Change it event-based so that it can consume the 'click'

	virtual void UpdateOutlineVertsForMe() override;
	virtual void UpdateTextVertsForMe() override;

private:
	std::string m_inputText;
	const float m_textCellHeight = 0.0f;
	const float m_textCellAspect = 0.0f;

	std::string m_defaultText;
	std::string m_pathCutoffSubstring;
};