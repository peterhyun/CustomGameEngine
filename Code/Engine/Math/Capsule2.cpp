#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/MathUtils.hpp"

Capsule2::Capsule2(const LineSegment2& bone, float radius)
: m_bone(bone), m_radius(radius), m_boundingSphereCenter((bone.m_start + bone.m_end) * 0.5f), m_boundingSphereRadius(radius + ((bone.m_end - bone.m_start) * 0.5f).GetLength())
{
}

Capsule2::Capsule2(const Vec2& start, const Vec2& end, float radius)
: m_bone(start, end), m_radius(radius), m_boundingSphereCenter((start + end) * 0.5f), m_boundingSphereRadius(radius + ((end - start) * 0.5f).GetLength())
{
}

void Capsule2::Translate(const Vec2& translation)
{
	m_bone.Translate(translation);
	m_boundingSphereCenter += translation;
}

void Capsule2::SetCenter(const Vec2& newCenter)
{
	m_bone.SetCenter(newCenter);
	m_boundingSphereCenter = newCenter;
}

void Capsule2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_bone.RotateAboutCenter(rotationDeltaDegrees);
}

void Capsule2::SetStartPos(const Vec2& startPos)
{
	m_bone.m_start = startPos;
	m_boundingSphereCenter = (startPos + m_bone.m_end) * 0.5f;
	m_boundingSphereRadius = m_radius + m_bone.GetLength() * 0.5f;
}

void Capsule2::SetEndPos(const Vec2& endPos)
{
	m_bone.m_end = endPos;
	m_boundingSphereCenter = (m_bone.m_start + endPos) * 0.5f;
	m_boundingSphereRadius = m_radius + m_bone.GetLength() * 0.5f;
}

void Capsule2::SetRadius(float radius)
{
	m_radius = radius;
	m_boundingSphereRadius = radius + m_bone.GetLength() * 0.5f;
}

float Capsule2::GetRadius() const
{
	return m_radius;
}

Vec2 Capsule2::GetStartPos() const
{
	return m_bone.m_start;
}

Vec2 Capsule2::GetEndPos() const
{
	return m_bone.m_end;
}

LineSegment2 Capsule2::GetLineSegment() const
{
	return m_bone;
}

Vec2 Capsule2::GetBoundingSphereCenter() const
{
	return m_boundingSphereCenter;
}

float Capsule2::GetBoundingSphereRadius() const
{
	return m_boundingSphereRadius;
}
