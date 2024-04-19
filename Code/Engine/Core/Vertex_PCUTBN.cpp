#include "Vertex_PCUTBN.hpp"

Vertex_PCUTBN::Vertex_PCUTBN()
{
}

Vertex_PCUTBN::Vertex_PCUTBN(Vec3 const& position, Vec3 const& normal, Rgba8 const& tint, Vec2 const& uvTexCoords)
	:m_position(position), m_normal(normal), m_color(tint), m_uvTexCoords(uvTexCoords)
{
}

Vertex_PCUTBN::Vertex_PCUTBN(Vec2 const& position, Vec3 const& normal, Rgba8 const& tint, Vec2 const& uvTexCoords)
	:m_position(position), m_normal(normal), m_color(tint), m_uvTexCoords(uvTexCoords)
{
}

Vertex_PCUTBN::Vertex_PCUTBN(Vec3 const& position, Rgba8 const& tint, Vec2 const& uvTexCoords, const Vec3& tangent, const Vec3& binormal, const Vec3& normal)
	: m_position(position), m_color(tint), m_uvTexCoords(uvTexCoords), m_tangent(tangent), m_binormal(binormal), m_normal(normal)
{
}

bool Vertex_PCUTBN::operator<(const Vertex_PCUTBN& other) const
{
	if (m_position != other.m_position) {
		return m_position < other.m_position;
	}
	if (m_color != other.m_color) {
		return m_color < other.m_color;
	}
	if (m_uvTexCoords != other.m_uvTexCoords) {
		return m_uvTexCoords < other.m_uvTexCoords;
	}
	if (m_tangent != other.m_tangent) {
		return m_tangent < other.m_tangent;
	}
	if (m_binormal != other.m_binormal) {
		return m_binormal < other.m_binormal;
	}
	return m_normal < other.m_normal;
}
