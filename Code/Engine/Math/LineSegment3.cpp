#include "Engine/Math/LineSegment3.hpp"
#include "Engine/Math/MathUtils.hpp"

LineSegment3::LineSegment3(const Vec3& start, const Vec3& end) : m_start(start), m_end(end)
{
}

void LineSegment3::Translate(const Vec3& translation)
{
	m_start += translation;
	m_end += translation;
}

void LineSegment3::SetCenter(const Vec3& newCenter)
{
	Vec3 currentCenter((m_start + m_end) * 0.5f);
	Translate(newCenter - currentCenter);
}

float LineSegment3::GetLength() const
{
	return (m_end - m_start).GetLength();
}

float LineSegment3::GetLengthSquared() const
{
	return (m_end - m_start).GetLengthSquared();
}
