#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include <string>
#include <functional>

class BehaviorTreeNode;
class Stopwatch;
class NamedProperties;
class BitmapFont;

typedef NamedProperties EventArgs;

constexpr float CARETSTOPWATCHTIME = 0.5f;

class NodeTextTypeBarComponent {
public:
	NodeTextTypeBarComponent(const BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs, float textCellHeight);
	~NodeTextTypeBarComponent();
	void UpdateInEditor();

	void SetText(const std::string& text);

	void AddOutlineVertsForMe(std::vector<Vertex_PCU>& out_verts);
	void AddTextVertsForMe(const BitmapFont& bitmapFont, std::vector<Vertex_PCU>& out_verts);

private:
	void UpdateFromKeyboard();
	bool Event_CharPressed(EventArgs& args);
	//bool Event_KeyPressed(EventArgs& args);

private:
	const BehaviorTreeNode& m_ownerNode;

	const Vec2 m_nodeUVMins;
	const Vec2 m_nodeUVMaxs;

	bool m_isActive = false;

	Stopwatch* m_caretStopwatch = nullptr;
	bool m_isCaretVisible = false;
	int m_caretPosition = 0;

	std::string m_inputText;
	float m_textCellHeight = 0.0f;

	std::function<bool(std::string)> m_enterKeyCallbackFunction = nullptr;
};