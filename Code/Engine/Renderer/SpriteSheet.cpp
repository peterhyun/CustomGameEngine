#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"

SpriteSheet::SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout): m_texture(texture), m_gridLayout(simpleGridLayout)
{
	int numSprites = simpleGridLayout.x * simpleGridLayout.y;
	m_spriteDefs.reserve(numSprites);

	float singleSpriteUVSpaceWidth = (1.0f / (float)simpleGridLayout.x);
	float singleSpriteUVSpaceHeight = (1.0f / (float)simpleGridLayout.y);

	IntVec2 textureDimensions = m_texture.GetDimensions();
	float textureUVOffsetX = (float)textureDimensions.x * 128.f;
	float textureUVOffsetY = (float)textureDimensions.y * 128.f;
	for (int spriteIndex = 0; spriteIndex < numSprites; spriteIndex++) {
		int sprite2DIndex_x = spriteIndex % simpleGridLayout.x;
		int sprite2DIndex_y = int(float(spriteIndex) * singleSpriteUVSpaceWidth);
		Vec2 uvAtMins = Vec2((singleSpriteUVSpaceWidth * sprite2DIndex_x) + (1.0f/textureUVOffsetX), 1.0f - (singleSpriteUVSpaceHeight * (sprite2DIndex_y + 1)) + (1.0f/textureUVOffsetX));
		Vec2 uvAtMaxs = Vec2((singleSpriteUVSpaceWidth * (sprite2DIndex_x + 1)) - (1.0f/textureUVOffsetX), 1.0f - (singleSpriteUVSpaceHeight * sprite2DIndex_y) - (1.0f/textureUVOffsetY));
		m_spriteDefs.push_back(SpriteDefinition(*this, spriteIndex, uvAtMins, uvAtMaxs));
	}
}

SpriteSheet::SpriteSheet(const SpriteSheet& other):m_gridLayout(other.m_gridLayout), m_spriteDefs(other.m_spriteDefs), m_texture(other.m_texture)
{
	for (int i = 0; i < m_spriteDefs.size(); i++) {
		m_spriteDefs[i].m_spriteSheet = this;
	}
}

SpriteSheet::SpriteSheet(const SpriteSheet&& other) noexcept : m_gridLayout(other.m_gridLayout), m_spriteDefs(other.m_spriteDefs), m_texture(other.m_texture)
{
	for (int i = 0; i < m_spriteDefs.size(); i++) {
		m_spriteDefs[i].m_spriteSheet = this;
	}
}

Texture& SpriteSheet::GetTexture() const
{
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return static_cast<int>(m_spriteDefs.size());
}

SpriteDefinition const& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	AABB2 spriteUVsAABB2 = m_spriteDefs[spriteIndex].GetUVs();
	out_uvAtMins = spriteUVsAABB2.m_mins;
	out_uvAtMaxs = spriteUVsAABB2.m_maxs;
}

AABB2 SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex].GetUVs();
}

AABB2 SpriteSheet::GetSpriteUVs(const IntVec2& spriteCoords) const
{
	return GetSpriteUVs(spriteCoords.y * m_gridLayout.x + spriteCoords.x);
}