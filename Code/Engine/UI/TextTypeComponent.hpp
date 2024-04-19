#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/UI/Component.hpp"
#include <vector>
#include <string>
#include <functional>

class Widget;
class VertexBuffer;
class Stopwatch;
class NamedProperties;

typedef NamedProperties EventArgs;

constexpr float CARETSTOPWATCHTIME = 0.5f;

class TextTypeComponent : public Component {
public:
	TextTypeComponent(class Renderer& renderer, const Widget& ownerWidget, const AABB2& screenAABB2, float textCellHeight);
	~TextTypeComponent();
	void Update() override;
	void Render() const override;

	void RegisterEnterKeyCallbackFunction(std::function<bool(std::string)> function);

private:
	void UpdateFromKeyboard();
	bool Event_CharPressed(EventArgs& args);
	//bool Event_KeyPressed(EventArgs& args);

private:
	class Renderer& m_renderer;
	class BitmapFont* m_bitmapFont = nullptr;
	const Widget& m_ownerWidget;

	const AABB2 m_screenAABB2;

	bool m_isActive = false;

	Stopwatch* m_caretStopwatch = nullptr;
	bool m_isCaretVisible = false;
	int m_caretPosition = 0;

	std::string m_inputText;
	float m_textCellHeight = 0.0f;

	std::function<bool(std::string)> m_enterKeyCallbackFunction = nullptr;
};