#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/UI/Component.hpp"
#include <vector>
#include <string>

class Widget;
class VertexBuffer;

class DropDownComponent : public Component {
public:
	DropDownComponent(class Renderer& renderer, const std::vector<std::string>& options, const Widget& ownerWidget, const AABB2& buttonScreenAABB2, bool isOpenByDefault, bool shouldRenderUp, const Vec2& eachOptionAABB2Dimension);
	~DropDownComponent();
	void Update() override;
	void Render() const override;
	std::string GetSelectedOptionString() const;
	void ResetSelectedOptionIdx();

private:
	class Renderer& m_renderer;
	class BitmapFont* m_bitmapFont;
	const Widget& m_ownerWidget;
	std::vector<std::string> m_options;
	bool m_isOpen = false;
	AABB2 m_buttonScreenAABB2;
	AABB2 m_fullBox;

	int m_hoveredOptionIdx = -1;
	int m_selectedOptionIdx = -1;

	const Vec2 m_eachOptionAABB2Dim;

	const bool m_shouldRenderUp = false;

	const float m_inv_numOptions = 0.0f;
};