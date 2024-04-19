#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
#include <string>

class Renderer;
class Overlay;
class VertexBuffer;
struct Vec2;

enum class WidgetType {
	INVALID = -1,
	IMAGE,
	TEXTBOX,
	BUTTON,
	VARIABLESBOX,
	TIMELINE,
	POPUPPLAYER,
	NUM
};

class Widget {
	friend class Overlay;
public:
	//TODO: This constructor should be private and the overlay function should make it for the user
	Widget(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Rgba8& backgroundTint, const std::string& name = "");
	virtual void Update() = 0;
	virtual void Render() const = 0;
	void SetModelMatrix(const Mat44& modelMatrix);
	void AddBorders(const Vec2& boxWithinMin, const Vec2& boxWithinMax, const Rgba8& borderColor);
	const Overlay& GetConstRefToOverlay() const;
	AABB2 GetBounds() const;

protected:
	virtual ~Widget();	//Overlay deletes it

protected:
	Renderer& m_renderer;
	Overlay& m_overlay;
	WidgetType m_widgetType = WidgetType::INVALID;
	VertexBuffer* m_nonTextVBO = nullptr;
	VertexBuffer* m_borderVBO = nullptr;
	Mat44 m_modelMatrix;
	Rgba8 m_backgroundTint;
	Rgba8 m_borderTint;
	AABB2 m_screenSpaceAABB2;
	std::string m_name;
	bool m_isDisabled = false;

	std::vector<Vertex_PCU> m_nonTextVerts;
	std::vector<Vertex_PCU> m_borderVerts;
};