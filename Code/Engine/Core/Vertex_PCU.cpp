#include "Vertex_PCU.hpp"

Vertex_PCU::Vertex_PCU() : m_position(0.f, 0.f, 0.f), m_color(255, 255, 255, 255), m_uvTexCoords(0.f, 0.f) {}
Vertex_PCU::Vertex_PCU(Vec3 const& position, Rgba8 const& tint, Vec2 const& uvTexCoords) : m_position(position), m_color(tint), m_uvTexCoords(uvTexCoords) {}
Vertex_PCU::Vertex_PCU(Vec2 const& position, Rgba8 const& tint, Vec2 const& uvTexCoords) : m_position(position, 0.0f), m_color(tint), m_uvTexCoords(uvTexCoords) {}

bool Vertex_PCU::operator<(const Vertex_PCU& other) const
{
	if (m_position != other.m_position) {
		return m_position < other.m_position;
	}
	if (m_color != other.m_color) {
		return m_color < other.m_color;
	}
	return m_uvTexCoords < other.m_uvTexCoords;
}
