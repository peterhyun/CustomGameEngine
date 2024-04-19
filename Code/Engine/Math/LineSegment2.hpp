#pragma once
#include "Engine/Math/Vec2.hpp"

struct LineSegment2 {
public:
	LineSegment2() {};
	explicit LineSegment2(const Vec2& start, const Vec2& end);
	~LineSegment2() {};
	void Translate(const Vec2& translation);
	void SetCenter(const Vec2& newCenter);
	void RotateAboutCenter(float rotationDeltaDegrees);
	float GetLength() const;

public:
	Vec2 m_start;
	Vec2 m_end;
};