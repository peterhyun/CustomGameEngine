#include "Engine/Fbx/Vertex_FBX.hpp"

Vertex_FBX::Vertex_FBX()
{
}

Vertex_FBX::Vertex_FBX(Vec3 const& position, Vec3 const& normal, Vec3 const& tangent, Vec3 const& binormal, Rgba8 const& tint, Vec2 const& uvTexCoords, IntVec4 const& jointIndices1, IntVec4 const& jointIndices2, Vec4 const& jointWeights1, Vec4 const& jointWeights2, int materialIdx)
	:m_position(position), m_normal(normal), m_tangent(tangent), m_binormal(binormal), m_color(tint), m_uvTexCoords(uvTexCoords), m_jointIndices1(jointIndices1), m_jointIndices2(jointIndices2), m_jointWeights1(jointWeights1), m_jointWeights2(jointWeights2), m_materialIdx(materialIdx)
{
}

Vertex_FBX::Vertex_FBX(Vec2 const& position, Vec3 const& normal, Vec3 const& tangent, Vec3 const& binormal, Rgba8 const& tint, Vec2 const& uvTexCoords, IntVec4 const& jointIndices1, IntVec4 const& jointIndices2, Vec4 const& jointWeights1, Vec4 const& jointWeights2, int materialIdx)
	:m_position(position), m_normal(normal), m_tangent(tangent), m_binormal(binormal), m_color(tint), m_uvTexCoords(uvTexCoords), m_jointIndices1(jointIndices1), m_jointIndices2(jointIndices2), m_jointWeights1(jointWeights1), m_jointWeights2(jointWeights2), m_materialIdx(materialIdx)
{
}

bool Vertex_FBX::operator<(const Vertex_FBX& other) const
{
	if (m_position != other.m_position) {
		return m_position < other.m_position;
	}
	if (m_normal != other.m_normal) {
		return m_normal < other.m_normal;
	}
	if (m_tangent != other.m_tangent) {
		return m_tangent < other.m_tangent;
	}
	if (m_color != other.m_color) {
		return m_color < other.m_color;
	}
	if (m_uvTexCoords != other.m_uvTexCoords) {
		return m_uvTexCoords < other.m_uvTexCoords;
	}
	if (m_jointIndices1 != other.m_jointIndices1) {
		return m_jointIndices1 < other.m_jointIndices1;
	}
	if (m_jointIndices2 != other.m_jointIndices2) {
		return m_jointIndices2 < other.m_jointIndices2;
	}
	if (m_jointWeights1 != other.m_jointWeights1) {
		return m_jointWeights1 < other.m_jointWeights1;
	}
	if (m_jointWeights2 != other.m_jointWeights2) {
		return m_jointWeights2 < other.m_jointWeights2;
	}
	return m_materialIdx < other.m_materialIdx;
}