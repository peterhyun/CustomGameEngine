#pragma once
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>

class Texture;

class SpriteSheet
{
public:
	explicit SpriteSheet(Texture& texture, IntVec2 const& simpleGridLayout);
	SpriteSheet(const SpriteSheet& other);
	SpriteSheet(const SpriteSheet&& other) noexcept;

	Texture&				GetTexture() const;
	int						GetNumSprites() const;
	SpriteDefinition const& GetSpriteDef(int spriteIndex) const;
	void					GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const;
	AABB2					GetSpriteUVs(int spriteIndex) const;
	AABB2					GetSpriteUVs(const IntVec2& spriteCoords) const;

protected:
	Texture&				m_texture;
	IntVec2					m_gridLayout;
	std::vector<SpriteDefinition>	m_spriteDefs;	//Be careful when you call a SpriteSheet copy constructor!!! SpriteDefinition has a const reference to (*this) which means it will be a garbage value if SpriteSheet is allocated on the Stack memory.
};