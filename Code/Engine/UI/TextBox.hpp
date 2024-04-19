#pragma once
#include "Engine/UI/Widget.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class Renderer;
class VBO;
class VertexBuffer;
class BitmapFont;

class TextBox : public Widget {
	friend class Overlay;
public:
	TextBox(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const std::string& text, float textSize, const Vec2& textAlignment, const Rgba8& fontTint, const Rgba8& backgroundTint, const std::string& name);
	virtual void SetText(const std::string& newText);

protected:
	virtual void Update() override;
	virtual void Render() const override;
	virtual ~TextBox();

protected:
	std::vector<Vertex_PCU> m_textVertices;
	VertexBuffer* m_textVBO;
	BitmapFont* m_bitmapFont;
	Rgba8 m_textTint;

private:
	std::string m_text;
	float m_textSize;
	const Vec2 m_textAlignment;
};