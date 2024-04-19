#pragma once
#include "Engine/Math/Vec3.hpp"

struct LineSegment3 {
public:
	LineSegment3() {};
	explicit LineSegment3(const Vec3& start, const Vec3& end);
	~LineSegment3() {};
	void Translate(const Vec3& translation);
	void SetCenter(const Vec3& newCenter);
	float GetLength() const;
	float GetLengthSquared() const;

public:
	Vec3 m_start;
	Vec3 m_end;
};