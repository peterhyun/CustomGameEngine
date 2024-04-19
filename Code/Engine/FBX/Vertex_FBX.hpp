#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec4.hpp"
#include "Engine/Core/Rgba8.hpp"

typedef struct Vertex_FBX {
public:
	Vertex_FBX();
	explicit Vertex_FBX(Vec3 const& position, Vec3 const& normal, Vec3 const& tangent, Vec3 const& binormal, Rgba8 const& tint = Rgba8(255, 255, 255, 255), Vec2 const& uvTexCoords = Vec2(0.f, 0.f), IntVec4 const& jointIndices1 = IntVec4(0, 0, 0, 0), IntVec4 const& jointIndices2 = IntVec4(0, 0, 0, 0), Vec4 const& weights1 = Vec4(0.0f, 0.0f, 0.0f, 0.0f), Vec4 const& weights2 = Vec4(0.0f, 0.0f, 0.0f, 0.0f), int materialIdx = 0);
	explicit Vertex_FBX(Vec2 const& position, Vec3 const& normal, Vec3 const& tangent, Vec3 const& binormal, Rgba8 const& tint = Rgba8(255, 255, 255, 255), Vec2 const& uvTexCoords = Vec2(0.f, 0.f), IntVec4 const& jointIndices1 = IntVec4(0, 0, 0, 0), IntVec4 const& jointIndices2 = IntVec4(0, 0, 0, 0), Vec4 const& weights1 = Vec4(0.0f, 0.0f, 0.0f, 0.0f), Vec4 const& weights2 = Vec4(0.0f, 0.0f, 0.0f, 0.0f), int materialIdx = 0);

	bool operator<(const Vertex_FBX& other) const;

public:
	Vec3 m_position;
	Vec3 m_normal;
	Vec3 m_tangent;
	Vec3 m_binormal;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;
	IntVec4 m_jointIndices1;
	IntVec4 m_jointIndices2;
	Vec4 m_jointWeights1;
	Vec4 m_jointWeights2;
	int  m_materialIdx = 0;
} Vertex_FBX;