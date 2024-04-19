#pragma once
#include "Engine/Math/LineSegment3.hpp"

struct Capsule3 {
public:
	Capsule3() {};
	explicit Capsule3(const LineSegment3& bone, float radius);
	explicit Capsule3(const Vec3& start, const Vec3& end, float radius);
	~Capsule3() {};
	void Translate(const Vec3& translation);
	void SetCenter(const Vec3& newCenter);
	void SetStartPos(const Vec3& startPos);
	void SetEndPos(const Vec3& endPos);
	void SetRadius(float radius);

	float GetRadius() const;
	Vec3 GetStartPos() const;
	Vec3 GetEndPos() const;
	LineSegment3 GetLineSegment() const;
	Vec3 GetBoundingSphereCenter() const;
	float GetBoundingSphereRadius() const;

private:
	LineSegment3 m_bone;
	float m_radius = 0.f;

	Vec3 m_boundingSphereCenter;
	float m_boundingSphereRadius = 0.0f;
};