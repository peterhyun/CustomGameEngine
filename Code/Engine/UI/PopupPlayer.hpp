#pragma once
#include "Engine/UI/Widget.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include <functional>

class Renderer;
class VBO;
class Texture;
struct AABB2;
class Overlay;

class PopupPlayer : public Widget {
	friend class Overlay;
public:
	PopupPlayer(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Rgba8& tint, const std::string& name);
	virtual void RegisterPopupRenderCallbackFunction(std::function<void()> function);

protected:
	virtual ~PopupPlayer();
	virtual void Update() override;
	virtual void Render() const override;

private:
	void UpdateFromKeyboard();

private:
	Vec2 m_offsetFromClickedMouseCursorToCenter;
	bool m_isDragged = false;

	std::function<void()> m_popupRenderCallbackFunction = nullptr;
};