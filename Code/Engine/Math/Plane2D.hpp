#pragma once
#include "Engine/Math/Vec2.hpp"

//Plane equation dot(P, N) = D
struct Plane2D {
public:
	Plane2D(const Vec2& normal, float distFromOrigin);

public:
	Vec2 m_normal;
	float m_distFromOrigin = 0.0f;
};