#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

typedef struct Vertex_PCUTBN {
public:
	Vertex_PCUTBN();
	explicit Vertex_PCUTBN(Vec3 const& position, Vec3 const& normal, Rgba8 const& tint = Rgba8(255, 255, 255, 255), Vec2 const& uvTexCoords = Vec2(0.f, 0.f));
	explicit Vertex_PCUTBN(Vec2 const& position, Vec3 const& normal, Rgba8 const& tint = Rgba8(255, 255, 255, 255), Vec2 const& uvTexCoords = Vec2(0.f, 0.f));
	explicit Vertex_PCUTBN(Vec3 const& position, Rgba8 const& tint = Rgba8(255, 255, 255, 255), Vec2 const& uvTexCoords = Vec2(0.f, 0.f), const Vec3& tangent = Vec3(), const Vec3& binormal = Vec3(), const Vec3& normal = Vec3());

	bool operator<(const Vertex_PCUTBN& other) const;

public:
	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;
	Vec3 m_tangent;
	Vec3 m_binormal;
	Vec3 m_normal;
} Vertex_PCUTBN;