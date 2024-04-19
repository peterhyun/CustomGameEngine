#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/CubicHermiteCurve2.hpp"
#include <vector>

struct CatmullRomSpline2
{
public:
	CatmullRomSpline2(const std::vector<Vec2>& keyFrames);
	Vec2 EvaluateAtParametric(float t) const;
	Vec2 EvaluateTangentAtParametric(float t) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;
	void ResetKeyFrames(const std::vector<Vec2>& keyFrames);
	int GetNumKeyFrames() const;
	Vec2 GetKeyFrame(int keyFrameIndex) const;

public:
	std::vector<CubicHermiteCurve2> m_cubicHermiteCurves;

private:
	std::vector<Vec2> m_keyFrames;
};