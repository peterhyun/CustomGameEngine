#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/MathUtils.hpp"

Capsule3::Capsule3(const LineSegment3& bone, float radius)
	: m_bone(bone), m_radius(radius), m_boundingSphereCenter((bone.m_start + bone.m_end) * 0.5f), m_boundingSphereRadius(radius + ((bone.m_end - bone.m_start) * 0.5f).GetLength())
{
}

Capsule3::Capsule3(const Vec3& start, const Vec3& end, float radius)
	: m_bone(start, end), m_radius(radius), m_boundingSphereCenter((start + end) * 0.5f), m_boundingSphereRadius(radius + ((end - start) * 0.5f).GetLength())
{
}

void Capsule3::Translate(const Vec3& translation)
{
	m_bone.Translate(translation);
	m_boundingSphereCenter += translation;
}

void Capsule3::SetCenter(const Vec3& newCenter)
{
	m_bone.SetCenter(newCenter);
	m_boundingSphereCenter = newCenter;
}

void Capsule3::SetStartPos(const Vec3& startPos)
{
	m_bone.m_start = startPos;
	m_boundingSphereCenter = (startPos + m_bone.m_end) * 0.5f;
	m_boundingSphereRadius = m_radius + m_bone.GetLength() * 0.5f;
}

void Capsule3::SetEndPos(const Vec3& endPos)
{
	m_bone.m_end = endPos;
	m_boundingSphereCenter = (m_bone.m_start + endPos) * 0.5f;
	m_boundingSphereRadius = m_radius + m_bone.GetLength() * 0.5f;
}

void Capsule3::SetRadius(float radius)
{
	m_radius = radius;
	m_boundingSphereRadius = radius + m_bone.GetLength() * 0.5f;
}

float Capsule3::GetRadius() const
{
	return m_radius;
}

Vec3 Capsule3::GetStartPos() const
{
	return m_bone.m_start;
}

Vec3 Capsule3::GetEndPos() const
{
	return m_bone.m_end;
}

LineSegment3 Capsule3::GetLineSegment() const
{
	return m_bone;
}

Vec3 Capsule3::GetBoundingSphereCenter() const
{
	return m_boundingSphereCenter;
}

float Capsule3::GetBoundingSphereRadius() const
{
	return m_boundingSphereRadius;
}
