#pragma once
#include "Engine/UI/TextBox.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <functional>

class Renderer;
class VBO;
struct AABB2;
struct Rgba8;
class Overlay;
struct Vec2;

class Button : public TextBox {
	friend class Overlay;
public:
	Button(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const std::string& buttonText, float buttonTextSize, const Vec2& textAlignment, const Rgba8& textNormalColor, const Rgba8& textHoveredColor, const Rgba8& buttonNormalColor, const Rgba8& buttonHoveredColor, const std::string& name);
	virtual void RegisterCallbackFunction(std::function<void()> function);

protected:
	virtual void SetIsFocused(bool isFocused);
	virtual void Update() override;
	virtual void Render() const override;

	virtual void OnClick();
	virtual ~Button();

private:
	const Rgba8 m_textHoveredColor;
	const Rgba8 m_buttonHoveredColor;
	bool m_isFocused = false;

	std::function<void()> m_registeredFunction = nullptr;
};