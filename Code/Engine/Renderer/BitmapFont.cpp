#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

BitmapFont::BitmapFont(const char* fontFilePathNameWithNoExtension, Texture& fontTexture) : m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension), m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{
}

Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

AABB2 BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect) const
{
	float cellWidth = cellHeight * cellAspect;
	AABB2 finalAABB2(textMins, textMins);
	for (int i = 0; i < text.length(); i++) {
		Vec2 uvAtMins;
		Vec2 uvAtMaxs;
		m_fontGlyphsSpriteSheet.GetSpriteUVs(uvAtMins, uvAtMaxs, text[i]);
		AABB2 tempAABB2(Vec2(textMins.x + cellWidth * i, textMins.y), Vec2(textMins.x + cellWidth * (i + 1), textMins.y + cellHeight));
		finalAABB2 = MergeAABB2s(finalAABB2, tempAABB2);
		AddVertsForAABB2(verts, tempAABB2, tint, AABB2(uvAtMins, uvAtMaxs));
	}
	return finalAABB2;
}

AddVertsForTextInBoxReturnInfo BitmapFont::AddVertsForTextInBox2D(std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextBoxMode mode, int maxGlyphsToDraw, bool shouldFixMaxsYOfBox) const
{
	std::vector<Vertex_PCU> tempVerts;

	std::string textWithLimitedGlyphs = text;
	for (int i = maxGlyphsToDraw; i < text.length(); i++) {
		if(textWithLimitedGlyphs[i] != '\n')
			textWithLimitedGlyphs[i] = ' ';
	}

	Strings splitStrings = SplitStringOnDelimeter(textWithLimitedGlyphs, '\n');

	GUARANTEE_OR_DIE(splitStrings.size() > 0, "splitStrings in AddVertsForTextInBox() should have at least 1 entry");

	Vec2 boxDimensions = box.GetDimensions();
	float totalCellsHeight = cellHeight * (float)splitStrings.size();
	
	//Get the longest string
	std::string* longestString = &splitStrings[0];
	for (int splitStringIdx = 1; splitStringIdx < splitStrings.size(); splitStringIdx++) {
		if (splitStrings[splitStringIdx].length() > longestString->length())
			longestString = &splitStrings[splitStringIdx];
	}
	float totalCellsWidth = GetTextWidth(cellHeight, *longestString, cellAspect);
	
	if (mode == TextBoxMode::SHRINK_TO_FIT) {
		if (totalCellsHeight > boxDimensions.y) {
			totalCellsHeight = boxDimensions.y;
			cellHeight = totalCellsHeight / (float)splitStrings.size();
			totalCellsWidth = GetTextWidth(cellHeight, *longestString, cellAspect);
		}
		if (totalCellsWidth > boxDimensions.x) {
			totalCellsHeight = totalCellsHeight * (boxDimensions.x / totalCellsWidth);
			cellHeight = totalCellsHeight / (float)splitStrings.size();
			totalCellsWidth = boxDimensions.x;
		}
	}

	//Start stacking strings (I precalculated the spltStringIdx = 0 version so that I can use finalBox)
	float textBoxOffsetX = (totalCellsWidth - GetTextWidth(cellHeight, splitStrings[0], cellAspect)) * alignment.x;
	float textBoxOffsetY = cellHeight * (splitStrings.size() - 1);
	Vec2 textBoxMin = box.m_mins + Vec2((boxDimensions.x - totalCellsWidth) * alignment.x, (boxDimensions.y - totalCellsHeight) * alignment.y);
	AABB2 finalBox = AddVertsForText2D(tempVerts, textBoxMin + Vec2(textBoxOffsetX, textBoxOffsetY), cellHeight, splitStrings[0], tint, cellAspect);
	for (int splitStringIdx = 1; splitStringIdx < splitStrings.size(); splitStringIdx++) {
		textBoxOffsetX = (totalCellsWidth - GetTextWidth(cellHeight, splitStrings[splitStringIdx], cellAspect)) * alignment.x;
		textBoxOffsetY = cellHeight * (splitStrings.size() - 1 - splitStringIdx);
		textBoxMin = box.m_mins + Vec2((boxDimensions.x - totalCellsWidth) * alignment.x, (boxDimensions.y - totalCellsHeight) * alignment.y);
		AABB2 tempBox = AddVertsForText2D(tempVerts, textBoxMin + Vec2(textBoxOffsetX, textBoxOffsetY), cellHeight, splitStrings[splitStringIdx], tint, cellAspect);
		finalBox = MergeAABB2s(finalBox, tempBox);
	}

	if (mode == TextBoxMode::OVERRUN && shouldFixMaxsYOfBox) {
		float amountToShiftDownwards = box.m_maxs.y - finalBox.m_maxs.y;
		finalBox.Translate(Vec2(0.0f, amountToShiftDownwards));
		for (int i = 0; i < tempVerts.size(); i++) {
			tempVerts[i].m_position.y += amountToShiftDownwards;
		}
	}

	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());

	AddVertsForTextInBoxReturnInfo returnInfo;
	returnInfo.m_requestedBox = box;
	returnInfo.m_boxThatBoundsAllTextVerts = finalBox;
	returnInfo.m_modifiedCellHeightForShrinkToFit = cellHeight;
	return returnInfo;
}

void BitmapFont::AddVertsForTextAtOriginXForward3D(std::vector<Vertex_PCU>& verts, Vec2 const& textMins, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect, const Vec2& alignment) const
{
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForText2D(tempVerts, textMins, cellHeight, text, tint, cellAspect);

	AABB2 textBounds2D = GetVertexBounds2D(tempVerts);
	
	//A coordinate is the original z forward text verts. B coordinate is after the translation accordingly to the alignment. C coordinate is after moving around the axis to x forward.

	//C coordinate System to B (moving axis order)
	Mat44 transform;
	transform.SetIJK3D(Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));

	//B coordinate system to A (Translation) B^-1 * A
	Mat44 BToA;
	BToA.SetTranslation2D(-textBounds2D.GetDimensions() * alignment);

	transform.Append(BToA);

	TransformVertexArray3D(tempVerts, transform);

	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

float BitmapFont::GetTextWidth(float cellHeight, std::string const& text, float cellAspect) const
{
	return (cellAspect * cellHeight) * text.length();
}

std::string BitmapFont::GetFontFilePathWithNoExtension() const
{
	return m_fontFilePathNameWithNoExtension;
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	(int)glyphUnicode;
	return 1.0f;
}
