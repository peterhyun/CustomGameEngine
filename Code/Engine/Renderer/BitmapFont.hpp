#pragma once
#include <vector>
#include <string>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Eulerangles.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

typedef enum class TextBoxMode {
	INVALID = -1, SHRINK_TO_FIT, OVERRUN, NUM_TEXTDRAWMODE
} TextBoxMode;

struct AddVertsForTextInBoxReturnInfo {
	AABB2 m_requestedBox;
	AABB2 m_boxThatBoundsAllTextVerts;
	float m_modifiedCellHeightForShrinkToFit = 0.0f;
};

class BitmapFont
{
	friend class Renderer;
private:
	BitmapFont(const char * fontFilePathNameWithNoExtension, Texture& fontTexture);
	BitmapFont(BitmapFont const& copy) = delete; // No copying allowed!  This represents GPU memory.
	~BitmapFont() {};

public:
	Texture& GetTexture();
	//Returns the AABB2 that contains the text
	AABB2 AddVertsForText2D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f) const;
	//Returns useful information
	AddVertsForTextInBoxReturnInfo AddVertsForTextInBox2D(std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f, Vec2 const& alignment = Vec2(0.5f, 0.5f), TextBoxMode mode = TextBoxMode::SHRINK_TO_FIT, int maxGlyphsToDraw = 99999999, bool shouldFixMaxsYOfBox = false) const;
	void AddVertsForTextAtOriginXForward3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f, const Vec2& alignment = Vec2(.5f, .5f)) const;
	float GetTextWidth(float cellHeight, std::string const& text, float cellAspect = 1.f) const;
	std::string GetFontFilePathWithNoExtension() const;

protected:
	float GetGlyphAspect(int glyphUnicode) const;

protected:
	std::string m_fontFilePathNameWithNoExtension;
	SpriteSheet m_fontGlyphsSpriteSheet;
};