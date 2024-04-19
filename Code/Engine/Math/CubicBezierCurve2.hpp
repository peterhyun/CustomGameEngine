#pragma once
#include "Engine/Math/Vec2.hpp"

struct CubicHermiteCurve2;

struct CubicBezierCurve2
{
public:
	CubicBezierCurve2(const Vec2& startPos, const Vec2& guidePos1, const Vec2& guidePos2, const Vec2& endPos);
	explicit CubicBezierCurve2(const CubicHermiteCurve2& fromHermite);
	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	Vec2 EvaluateTangentAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;

public:
	Vec2 m_startPos;
	Vec2 m_guidePos1;
	Vec2 m_guidePos2;
	Vec2 m_endPos;
};