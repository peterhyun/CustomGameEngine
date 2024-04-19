#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/MathUtils.hpp"

LineSegment2::LineSegment2(const Vec2& start, const Vec2& end): m_start(start), m_end(end)
{
}

void LineSegment2::Translate(const Vec2& translation)
{
	m_start += translation;
	m_end += translation;
}

void LineSegment2::SetCenter(const Vec2& newCenter)
{
	Vec2 currentCenter((m_start + m_end) * 0.5f);
	Translate(newCenter - currentCenter);
}

void LineSegment2::RotateAboutCenter(float rotationDeltaDegrees)
{
	Vec2 currentCenter((m_start + m_end) * 0.5f);
	Vec2 fromCenterToEnd = m_end - currentCenter;
	TransformPosition2D(fromCenterToEnd, 1.0f, rotationDeltaDegrees, Vec2(0.0f, 0.0f));
	m_start = currentCenter - 0.5f * fromCenterToEnd;
	m_end = currentCenter + 0.5f * fromCenterToEnd;
}

float LineSegment2::GetLength() const
{
	return (m_end - m_start).GetLength();
}
