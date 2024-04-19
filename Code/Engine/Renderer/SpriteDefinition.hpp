#pragma once
#include "Engine/Math/AABB2.hpp"

class SpriteSheet;
class Texture;

class SpriteDefinition
{
public:
	friend class SpriteSheet;
	explicit				SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs);
	void					GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const;
	AABB2					GetUVs() const;
	SpriteSheet const&		GetSpriteSheet() const;
	Texture&				GetTexture() const;
	float					GetAspect() const;

protected:
	const SpriteSheet *		m_spriteSheet;
	int						m_spriteIndex	= -1;
	Vec2					m_uvAtMins		= Vec2::ZERO;
	Vec2					m_uvAtMaxs		= Vec2::ONE;
};