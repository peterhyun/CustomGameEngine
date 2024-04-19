#include "Engine/Math/CubicBezierCurve2.hpp"
#include "Engine/Math/CubicHermiteCurve2.hpp"
#include "Engine/Math/MathUtils.hpp"

CubicBezierCurve2::CubicBezierCurve2(const Vec2& startPos, const Vec2& guidePos1, const Vec2& guidePos2, const Vec2& endPos)
	:m_startPos(startPos), m_guidePos1(guidePos1), m_guidePos2(guidePos2), m_endPos(endPos)
{
}

CubicBezierCurve2::CubicBezierCurve2(const CubicHermiteCurve2& fromHermite)
	:m_startPos(fromHermite.m_startPos), m_endPos(fromHermite.m_endPos),
	m_guidePos1(fromHermite.m_startTangent / 3.0f + fromHermite.m_startPos), 
	m_guidePos2(fromHermite.m_endPos - fromHermite.m_endTangent / 3.0f)
{
}

Vec2 CubicBezierCurve2::EvaluateAtParametric(float parametricZeroToOne) const
{
	return Vec2(ComputeCubicBezier1D(m_startPos.x, m_guidePos1.x, m_guidePos2.x, m_endPos.x, parametricZeroToOne), 
		ComputeCubicBezier1D(m_startPos.y, m_guidePos1.y, m_guidePos2.y, m_endPos.y, parametricZeroToOne));
}

Vec2 CubicBezierCurve2::EvaluateTangentAtParametric(float parametricZeroToOne) const
{
	return Vec2(ComputeCubicBezierTangent1D(m_startPos.x, m_guidePos1.x, m_guidePos2.x, m_endPos.x, parametricZeroToOne),
		ComputeCubicBezierTangent1D(m_startPos.y, m_guidePos1.y, m_guidePos2.y, m_endPos.y, parametricZeroToOne));
}

float CubicBezierCurve2::GetApproximateLength(int numSubdivisions) const
{
	float invSubdivisions = 1.0f / (float)numSubdivisions;
	Vec2 previousResult = m_startPos;
	float length = 0.0f;
	for (int i = 1; i <= numSubdivisions ; i++) {
		Vec2 currentPosOnCurve = EvaluateAtParametric(invSubdivisions * (float)i);
		length += (currentPosOnCurve - previousResult).GetLength();
		previousResult = currentPosOnCurve;
	}
	return length;
}

Vec2 CubicBezierCurve2::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
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
