#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"

const AABB3 AABB3::ZERO_TO_ONE(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

AABB3::~AABB3()
{
}

AABB3::AABB3()
{
}

AABB3::AABB3(AABB3 const& copyFrom) : m_mins(copyFrom.m_mins), m_maxs(copyFrom.m_maxs)
{
}

AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) : m_mins(minX, minY, minZ), m_maxs(maxX, maxY, maxZ)
{
}

AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs):m_mins(mins), m_maxs(maxs)
{
}

AABB3::AABB3(Vec3 const& center, float xHalfDimension, float yHalfDimension, float zHalfDimension)
	: m_mins(center.x - xHalfDimension, center.y - yHalfDimension, center.z - zHalfDimension), m_maxs(center.x + xHalfDimension, center.y + yHalfDimension, center.z + zHalfDimension)
{
}

Vec3 AABB3::GetFLB() const
{
	return Vec3(m_mins.x, m_maxs.y, m_mins.z);
}

Vec3 AABB3::GetFLT() const
{
	return Vec3(m_mins.x, m_maxs.y, m_maxs.z);
}

Vec3 AABB3::GetFRT() const
{
	return Vec3(m_mins.x, m_mins.y, m_maxs.z);
}

Vec3 AABB3::GetBLB() const
{
	return Vec3(m_maxs.x, m_maxs.y, m_mins.z);
}

Vec3 AABB3::GetBRT() const
{
	return Vec3(m_maxs.x, m_mins.y, m_maxs.z);
}

Vec3 AABB3::GetBRB() const
{
	return Vec3(m_maxs.x, m_mins.y, m_mins.z);
}

Vec3 AABB3::GetCenter() const
{
	return (m_mins + m_maxs) * 0.5f;
}

bool AABB3::IsPointInside(const Vec3& point) const
{
	return m_mins.x <= point.x && m_mins.y <= point.y && m_mins.z <= point.z &&
		point.x <= m_maxs.x && point.y <= m_maxs.y && point.z <= m_maxs.z;
}

AABB2 AABB3::GetBoundsXY() const
{
	return AABB2(Vec2(m_mins), Vec2(m_maxs));
}

AABB3 AABB3::GetBoundsXY2D() const
{
	return AABB3(Vec3(m_mins.x, m_mins.y, 0.0f), Vec3(m_maxs.x, m_maxs.y, 0.0f));
}

float AABB3::GetBoundingRadius() const
{
	return (m_maxs - m_mins).GetLength() * 0.5f;
}

Vec3 AABB3::GetNearestPoint(const Vec3& referencePosition) const
{
	return Vec3(GetClamped(referencePosition.x, m_mins.x, m_maxs.x), GetClamped(referencePosition.y, m_mins.y, m_maxs.y), GetClamped(referencePosition.z, m_mins.z, m_maxs.z));
}

void AABB3::StretchToIncludePoint(Vec3 const& point)
{
	if (IsPointInside(point)) {
		return;
	}
	else {
		// Check and adjust for the x dimension
		if (point.x < m_mins.x) {
			m_mins.x = point.x;
		}
		else if (point.x > m_maxs.x) {
			m_maxs.x = point.x;
		}

		// Check and adjust for the y dimension
		if (point.y < m_mins.y) {
			m_mins.y = point.y;
		}
		else if (point.y > m_maxs.y) {
			m_maxs.y = point.y;
		}

		// Check and adjust for the z dimension
		if (point.z < m_mins.z) {
			m_mins.z = point.z;
		}
		else if (point.z > m_maxs.z) {
			m_maxs.z = point.z;
		}
	}
}

float AABB3::GetShortestDimensionSize()
{
	Vec3 dimensions = m_maxs - m_mins;
	float x = dimensions.x;
	float y = dimensions.y;
	float z = dimensions.z;

	if (x <= y) {
		if (x <= z) {
			return x;
		}
		else {
			return z;
		}
	}
	else {
		if (y <= z) {
			return y;
		}
		else {
			return z;
		}
	}
}
