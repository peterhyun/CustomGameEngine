#pragma once
#include "Engine/UI/Widget.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class Texture;
class VertexBuffer;
class Renderer;
class Button;

//Has a children of widgets...

class Overlay {
public:
	Overlay(Renderer& renderer, const AABB2& clientSpaceAABB2, const Rgba8& tint); //clientSpaceAABB should cover the entire screen! (Should be world bounds)
	~Overlay();
	void Update();
	void Render() const;
	AABB2 GetBounds() const;
	void AddWidget(Widget& widget);
	void FocusOnPreviousButton();
	void FocusOnNextButton();
	void ClickFocusedButton();
	Widget* GetWidgetFromName(const std::string& name) const;
	void SetWhetherWidgetIsDisabledFromName(const std::string& name, bool isDisabled);

private:
	AABB2 m_bounds;
	std::vector<Widget*> m_widgets;
	std::vector<Widget*> m_widgetsByType[(int)WidgetType::NUM];
	std::vector<Vertex_PCU> m_quadVerts;
	VertexBuffer* m_quadVBO = nullptr;
	Renderer& m_renderer;

	//Button* m_selectedButton = nullptr;
	int m_selectedButtonIdx = -1;
};