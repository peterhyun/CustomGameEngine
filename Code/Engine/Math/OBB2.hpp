#pragma once
#include "Engine/Math/Vec2.hpp"

struct OBB2 {
public:
	OBB2() {};
	explicit OBB2(const Vec2& center, const Vec2& iBasisNormal, const Vec2& halfDimensions);
	~OBB2() {};
	//Starting top-left, counter-clockwise order
	void GetCornerPoints(Vec2* out_fourCornerWorldPositions) const;
	Vec2 GetLocalPosForWorldPos(const Vec2& worldPos) const;
	Vec2 GetWorldPosForLocalPos(const Vec2& localPos) const;
	void RotateAboutCenter(float rotationDeltaDegrees);
	void SetHalfDimensions(const Vec2& halfDimensions);
	void SetIBasisNormal(const Vec2& iBasisNormal);
	void SetCenter(const Vec2& center);

public:
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDimensions;

	float m_boundingSphereRadius = 0.0f;
};