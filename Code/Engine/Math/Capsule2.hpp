#pragma once
#include "Engine/Math/LineSegment2.hpp"

struct Capsule2 {
public:
	Capsule2() {};
	explicit Capsule2(const LineSegment2& bone, float radius);
	explicit Capsule2(const Vec2& start, const Vec2& end, float radius);
	~Capsule2() {};
	void Translate(const Vec2& translation);
	void SetCenter(const Vec2& newCenter);
	void RotateAboutCenter(float rotationDeltaDegrees);
	void SetStartPos(const Vec2& startPos);
	void SetEndPos(const Vec2& endPos);
	void SetRadius(float radius);
	
	float GetRadius() const;
	Vec2 GetStartPos() const;
	Vec2 GetEndPos() const;
	LineSegment2 GetLineSegment() const;
	Vec2 GetBoundingSphereCenter() const;
	float GetBoundingSphereRadius() const;

private:
	LineSegment2 m_bone;
	float m_radius = 0.f;

	Vec2 m_boundingSphereCenter;
	float m_boundingSphereRadius = 0.0f;
};