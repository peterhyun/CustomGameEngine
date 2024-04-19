#include "AABB2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

const AABB2 AABB2::ZERO_TO_ONE(0.0f, 0.0f, 1.0f, 1.0f);

AABB2::AABB2(AABB2 const& copyFrom):m_mins(copyFrom.m_mins), m_maxs(copyFrom.m_maxs)
{
}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
{
	//GUARANTEE_OR_DIE(minX <= maxX && minY <= maxY, "AABB2 parameters invalid!");
	m_mins.x = minX;
	m_mins.y = minY;
	m_maxs.x = maxX;
	m_maxs.y = maxY;
}

AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs) :m_mins(mins), m_maxs(maxs)
{
	//GUARANTEE_OR_DIE(mins.x <= maxs.x && mins.y <= maxs.y, "AABB2 parameters invalid!");
}

bool AABB2::IsPointInside(Vec2 const& point) const
{
	return (point.x <= m_maxs.x) && (point.y <= m_maxs.y) && (point.x >= m_mins.x) && (point.y >= m_mins.y);
}

bool AABB2::IsAABB2Inside(AABB2 const& otherAABB2) const
{
	return (IsPointInside(otherAABB2.m_mins) && IsPointInside(otherAABB2.m_maxs) && IsPointInside(otherAABB2.GetTopLeft()) && IsPointInside(otherAABB2.GetBottomRight()));
}

Vec2 const AABB2::GetCenter() const
{
	return (m_mins + m_maxs) * 0.5f;
}

Vec2 const AABB2::GetDimensions() const
{
	return m_maxs - m_mins;
}

Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
	return Vec2(GetClamped(referencePosition.x, m_mins.x, m_maxs.x), GetClamped(referencePosition.y, m_mins.y, m_maxs.y));
}

Vec2 const AABB2::GetPointAtUV(Vec2 const& uv) const
{
	return Vec2(RangeMap(uv.x, 0.f, 1.f, m_mins.x, m_maxs.x), RangeMap(uv.y, 0.f, 1.f, m_mins.y, m_maxs.y));
}

Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
	return Vec2(GetFractionWithinRange(point.x, m_mins.x, m_maxs.x), GetFractionWithinRange(point.y, m_mins.y, m_maxs.y));
}

const AABB2 AABB2::GetBoxWithin(const Vec2& uvMins, const Vec2& uvMaxs) const
{
	return AABB2(GetPointAtUV(uvMins), GetPointAtUV(uvMaxs));
}

const AABB2 AABB2::GetLargestSquareWithin(const Vec2& centerPosUV) const
{
	AABB2 largestSquare;
	Vec2 centerPos = GetPointAtUV(centerPosUV);
	largestSquare.SetCenter(centerPos);
	float squareLength = 2.0f * GetMin(GetMin(GetMin(centerPos.x - m_mins.x, centerPos.y - m_mins.y), m_maxs.x - centerPos.x), m_maxs.y - centerPos.y);
	largestSquare.SetDimensions(Vec2(squareLength, squareLength));
	return largestSquare;
}

Vec2 const AABB2::GetTopLeft() const
{
	return Vec2(m_mins.x, m_maxs.y);
}

Vec2 const AABB2::GetBottomRight() const
{
	return Vec2(m_maxs.x, m_mins.y);
}

void AABB2::Translate(Vec2 const& translationToApply)
{
	m_mins += translationToApply;
	m_maxs += translationToApply;
}

void AABB2::SetCenter(Vec2 const& newCenter)
{
	Vec2 originalCenter = GetCenter();
	Vec2 translationVec2 = newCenter - originalCenter;
	Translate(translationVec2);
}

void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	Vec2 currentDimensions = GetDimensions();
	Vec2 dimensionsDiffVec = newDimensions - currentDimensions;
	m_mins -= dimensionsDiffVec * 0.5f;
	m_maxs += dimensionsDiffVec * 0.5f;
}

void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	if (IsPointInside(point)) {
		return;
	}
	else {
		if (point.x < m_mins.x)
			if (point.y > m_maxs.y) {
				m_mins.x = point.x;
				m_maxs.y = point.y;
			}
			else if (point.y < m_mins.y) {
				m_mins.x = point.x;
				m_mins.y = point.y;
			}
			else {
				m_mins.x = point.x;
			}
		else if (point.x > m_maxs.x) {
			if (point.y > m_maxs.y) {
				m_maxs.x = point.x;
				m_maxs.y = point.y;
			}
			else if (point.y < m_mins.y) {
				m_maxs.x = point.x;
				m_mins.y = point.y;
			}
			else {
				m_maxs.x = point.x;
			}
		}
		if (point.y < m_mins.y)
			m_mins.y = point.y;
		else if (point.y > m_maxs.y)
			m_maxs.y = point.y;
	}
}

AABB2 AABB2::FixBoxWithinThis(const AABB2& smallerBoxToFit)
{
	Vec2 smallBoxDim = smallerBoxToFit.GetDimensions();
	Vec2 thisDim = GetDimensions();
	GUARANTEE_OR_DIE(smallBoxDim.x <= thisDim.x && smallBoxDim.y <= thisDim.y, "Box cannot fit inside in the first place");
	Vec2 newMins = smallerBoxToFit.m_mins, newMaxs = smallerBoxToFit.m_maxs;
	if (smallerBoxToFit.m_mins.x < m_mins.x) {
		newMins.x = m_mins.x;
		newMaxs.x = newMins.x + smallBoxDim.x;
	}
	if (smallerBoxToFit.m_mins.y < m_mins.y) {
		newMins.y = m_mins.y;
		newMaxs.y = newMins.y + smallBoxDim.y;
	}
	if (smallerBoxToFit.m_maxs.x > m_maxs.x) {
		newMaxs.x = m_maxs.x;
		newMins.x = newMaxs.x - smallBoxDim.x;
	}
	if (smallerBoxToFit.m_maxs.y > m_maxs.y) {
		newMaxs.y = m_maxs.y;
		newMins.y = newMaxs.y - smallBoxDim.y;
	}
	return AABB2(newMins, newMaxs);
}

float AABB2::GetLargerDimensionSize()
{
	Vec2 dim = m_maxs - m_mins;;
	if (dim.x >= dim.y)
		return dim.x;
	else
		return dim.y;
}

float AABB2::GetShorterDimensionSize()
{
	Vec2 dim = m_maxs - m_mins;;
	if (dim.x >= dim.y)
		return dim.y;
	else
		return dim.x;
}

bool AABB2::operator==(const AABB2& other) const
{
	return (m_mins == other.m_mins) && (m_maxs == other.m_maxs);
}
