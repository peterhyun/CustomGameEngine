#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>

struct ConvexPoly2D {
public:
	ConvexPoly2D();
	ConvexPoly2D(std::vector<Vec2> ccwOrderedPoints);
	Vec2 GetCenterPos() const;
	void SetCenterPos(const Vec2& newCenterPos);

public:
	std::vector<Vec2> m_points;
};