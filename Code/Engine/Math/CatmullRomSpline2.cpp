#include "Engine/Math/CatmullRomSpline2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

CatmullRomSpline2::CatmullRomSpline2(const std::vector<Vec2>& keyFrames) : m_keyFrames(keyFrames)
{
	if (keyFrames.size() < 2) {
		ERROR_AND_DIE("There should be at least 2 keyframes for Catmull rom spline!!");
	}
	if (keyFrames.size() == 2) {
		m_cubicHermiteCurves.push_back(
			CubicHermiteCurve2(keyFrames[0], Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), keyFrames[1])
		);
		return;
	}
	m_cubicHermiteCurves.push_back(
		CubicHermiteCurve2(keyFrames[0], Vec2(0.0f, 0.0f), (keyFrames[2] - keyFrames[0]) * 0.5f, keyFrames[1])
	);
	for (int i = 1; i < keyFrames.size() - 2; i++) {
		m_cubicHermiteCurves.push_back(
			CubicHermiteCurve2(keyFrames[i], 
				(keyFrames[i + 1] - keyFrames[i-1]) * 0.5f, 
				(keyFrames[i + 2] - keyFrames[i]) * 0.5f, 
				keyFrames[i + 1])
		);
	}
	m_cubicHermiteCurves.push_back(
		CubicHermiteCurve2(keyFrames[keyFrames.size() - 2], 
			(keyFrames[keyFrames.size() - 1] - keyFrames[keyFrames.size() - 3]) * 0.5f, 
			Vec2(0.0f, 0.0f), 
			keyFrames[keyFrames.size() - 1])
	);
}

Vec2 CatmullRomSpline2::EvaluateAtParametric(float t) const
{
	if (t < 0) {
		ERROR_AND_DIE("t should NOT be smaller than 0 for a CatmullRomSpline");
	}
	int hermiteCurveIndex = (int)t;
	float tValueForLocalHermiteCurve = t - (float)hermiteCurveIndex;
	return m_cubicHermiteCurves[hermiteCurveIndex].EvaluateAtParametric(tValueForLocalHermiteCurve);
}

Vec2 CatmullRomSpline2::EvaluateTangentAtParametric(float t) const
{
	if (t < 0) {
		ERROR_AND_DIE("t should NOT be smaller than 0 for a CatmullRomSpline");
	}
	int hermiteCurveIndex = (int)t;
	float tValueForLocalHermiteCurve = t - (float)hermiteCurveIndex;
	return m_cubicHermiteCurves[hermiteCurveIndex].EvaluateTangentAtParametric(tValueForLocalHermiteCurve);
}

float CatmullRomSpline2::GetApproximateLength(int numSubdivisions) const
{
	float length = 0.0f;
	for (int i = 0; i < m_cubicHermiteCurves.size() ; i++) {
		length += m_cubicHermiteCurves[i].GetApproximateLength(numSubdivisions);
	}
	return length;
}

Vec2 CatmullRomSpline2::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	float distanceLeft = distanceAlongCurve;
	int indexHermiteCurve = 0;
	while (true) {
		float lengthForThisHermiteCurve = m_cubicHermiteCurves[indexHermiteCurve].GetApproximateLength(numSubdivisions);
		if (distanceLeft > lengthForThisHermiteCurve)
			distanceLeft -= lengthForThisHermiteCurve;
		else
			break;
		indexHermiteCurve++;
		if (indexHermiteCurve >= m_cubicHermiteCurves.size())
			indexHermiteCurve = 0;
	}
	return m_cubicHermiteCurves[indexHermiteCurve].EvaluateAtApproximateDistance(distanceLeft, numSubdivisions);
}

void CatmullRomSpline2::ResetKeyFrames(const std::vector<Vec2>& keyFrames)
{
	if (keyFrames.size() < 2) {
		ERROR_AND_DIE("There should be at least 2 keyframes for Catmull rom spline!!");
	}
	m_keyFrames = keyFrames;
	m_cubicHermiteCurves.clear();
	if (keyFrames.size() == 2) {
		m_cubicHermiteCurves.push_back(
			CubicHermiteCurve2(keyFrames[0], Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), keyFrames[1])
		);
		return;
	}
	m_cubicHermiteCurves.push_back(
		CubicHermiteCurve2(keyFrames[0], Vec2(0.0f, 0.0f), (keyFrames[2] - keyFrames[0]) * 0.5f, keyFrames[1])
	);
	for (int i = 1; i < keyFrames.size() - 2; i++) {
		m_cubicHermiteCurves.push_back(
			CubicHermiteCurve2(keyFrames[i],
				(keyFrames[i + 1] - keyFrames[i - 1]) * 0.5f,
				(keyFrames[i + 2] - keyFrames[i]) * 0.5f,
				keyFrames[i + 1])
		);
	}
	m_cubicHermiteCurves.push_back(
		CubicHermiteCurve2(keyFrames[keyFrames.size() - 2],
			(keyFrames[keyFrames.size() - 1] - keyFrames[keyFrames.size() - 3]) * 0.5f,
			Vec2(0.0f, 0.0f),
			keyFrames[keyFrames.size() - 1])
	);
}

int CatmullRomSpline2::GetNumKeyFrames() const
{
	return (int)m_keyFrames.size();
}

Vec2 CatmullRomSpline2::GetKeyFrame(int keyFrameIndex) const
{
	if (keyFrameIndex < 0 || keyFrameIndex >= m_keyFrames.size()) {
		ERROR_AND_DIE(Stringf("CatmullRomSpline2::GetKeyFrame() called with invalid keyFrameIndex: %d", keyFrameIndex));
	}
	return m_keyFrames[keyFrameIndex];
}