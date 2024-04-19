#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees):m_yawDegrees(yawDegrees), m_pitchDegrees(pitchDegrees), m_rollDegrees(rollDegrees)
{
}

EulerAngles::EulerAngles(const Vec3& angles) : m_yawDegrees(angles.x), m_pitchDegrees(angles.y), m_rollDegrees(angles.z)
{
}

void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float sr = SinDegrees(m_rollDegrees);
	out_forwardIBasis = Vec3(cy*cp, sy*cp, -sp);
	out_leftJBasis = Vec3(cy*sp*sr - sy*cr,
		sy*sp*sr + cy*cr,
		cp * sr);
	out_upKBasis = Vec3(cy * sp * cr + sy * sr,
		sy * sp * cr - cy * sr,
		cp * cr);
}

Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float sr = SinDegrees(m_rollDegrees);
	float matInput[16] = {
		cy* cp,
		sy* cp,
		-sp,
		0.0f,
		cy* sp* sr - sy * cr,
		sy* sp* sr + cy * cr,
		cp* sr,
		0.0f,
		cy* sp* cr + sy * sr,
		sy* sp* cr - cy * sr,
		cp* cr,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};
	return Mat44(matInput);
}

Vec3 EulerAngles::GetFwdVector_XFwd_YLeft_ZUp() const
{
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	return Vec3(cy * cp, sy * cp, -sp);
}

void EulerAngles::SetFromText(const char* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, ',');
	if (splitStrings.size() < 3)
		ERROR_RECOVERABLE("Setting EulerAngles from text but string contains less than 3 floats");
	m_yawDegrees = (float)atof(splitStrings[0].c_str());
	m_pitchDegrees = (float)atof(splitStrings[1].c_str());
	m_rollDegrees = (float)atof(splitStrings[2].c_str());
}
