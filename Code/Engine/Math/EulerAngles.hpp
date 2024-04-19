#pragma once
struct Vec3;
struct Mat44;

struct EulerAngles
{
public:
	EulerAngles() = default;
	EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees);
	EulerAngles(const Vec3& angles);
	void GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const;
	Mat44 GetAsMatrix_XFwd_YLeft_ZUp() const;
	Vec3 GetFwdVector_XFwd_YLeft_ZUp() const;
	void SetFromText(const char * text);
public:
	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
};