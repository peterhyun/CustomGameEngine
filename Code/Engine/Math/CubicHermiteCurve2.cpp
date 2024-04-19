#include "Engine/Math/CubicHermiteCurve2.hpp"
#include "Engine/Math/CubicBezierCurve2.hpp"
#include "Engine/Math/MathUtils.hpp"

CubicHermiteCurve2::CubicHermiteCurve2(const Vec2& startPos, const Vec2& startTangent, const Vec2& endTangent, const Vec2& endPos)
	:m_startPos(startPos), m_startTangent(startTangent), m_endTangent(endTangent), m_endPos(endPos)
{
}

CubicHermiteCurve2::CubicHermiteCurve2(const CubicBezierCurve2& fromBezier)
	:m_startPos(fromBezier.m_startPos), m_endPos(fromBezier.m_endPos),
	m_startTangent(3.0f * (fromBezier.m_guidePos1 - fromBezier.m_startPos)),
	m_endTangent(3.0f * (fromBezier.m_endPos - fromBezier.m_guidePos2))
{
}

Vec2 CubicHermiteCurve2::EvaluateAtParametric(float parametricZeroToOne) const
{
	float t = parametricZeroToOne;
	float t2 = parametricZeroToOne * parametricZeroToOne;
	float t3 = t2 * parametricZeroToOne;
	return
		(2.0f * t3 - 3.0f * t2 + 1.0f) * m_startPos +
		(t3 - 2.0f * t2 + t) * m_startTangent +
		(t3 - t2) * m_endTangent + 
		(-2.0f * t3 + 3.0f * t2) * m_endPos;
}

Vec2 CubicHermiteCurve2::EvaluateTangentAtParametric(float parametricZeroToOne) const
{
	float t = parametricZeroToOne;
	float t2 = parametricZeroToOne * parametricZeroToOne;

	return
		(6.0f * t2 - 6.0f * t) * m_startPos +
		(3.0f * t2 - 4.0f * t + 1.0f) * m_startTangent +
		(-6.0f * t2 + 6.0f * t) * m_endPos +
		(3.0f * t2 - 2.0f * t) * m_endTangent;
}

float CubicHermiteCurve2::GetApproximateLength(int numSubdivisions) const
{
	float invSubdivisions = 1.0f / (float)numSubdivisions;
	Vec2 previousResult = m_startPos;
	float length = 0.0f;
	for (int i = 1; i <= numSubdivisions; i++) {
		Vec2 currentPosOnCurve = EvaluateAtParametric(invSubdivisions * (float)i);
		length += (currentPosOnCurve - previousResult).GetLength();
		previousResult = currentPosOnCurve;
	}
	return length;
}

Vec2 CubicHermiteCurve2::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	float distanceLeft = distanceAlongCurve;
	float invSubdivisions = 1.0f / (float)numSubdivisions;
	Vec2 previousResult = m_startPos;
	Vec2 currentPosOnCurve;
	float length = 0.0f;
	for (int i = 1; i <= numSubdivisions; i++) {
		currentPosOnCurve = EvaluateAtParametric(invSubdivisions * (float)i);
		length = (currentPosOnCurve - previousResult).GetLength();
		if (distanceLeft > length)
			distanceLeft -= length;
		else
			break;
		previousResult = currentPosOnCurve;
	}
	return Lerp(previousResult, currentPosOnCurve, (distanceLeft / length));
}

void CubicHermiteCurve2::ResetValues(const Vec2& startPos, const Vec2& startTangent, const Vec2& endTangent, const Vec2& endPos)
{
	m_startPos = startPos;
	m_startTangent = startTangent;
	m_endTangent = endTangent;
	m_endPos = endPos;
}
