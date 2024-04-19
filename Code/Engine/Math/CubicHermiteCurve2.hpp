#pragma once
#include "Engine/Math/Vec2.hpp"

struct CubicBezierCurve2;

struct CubicHermiteCurve2
{
public:
	CubicHermiteCurve2(const Vec2& startPos, const Vec2& startTangent, const Vec2& endTangent, const Vec2& endPos);
	explicit CubicHermiteCurve2(const CubicBezierCurve2& fromBezier);
	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	Vec2 EvaluateTangentAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;
	void ResetValues(const Vec2& startPos, const Vec2& startTangent, const Vec2& endTangent, const Vec2& endPos);

public:
	Vec2 m_startPos;
	Vec2 m_startTangent;
	Vec2 m_endTangent;
	Vec2 m_endPos;
};