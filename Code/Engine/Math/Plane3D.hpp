#pragma once
#include "Engine/Math/Vec3.hpp"

//Plane equation dot(P, N) = D
struct Plane3D {
public:
	Plane3D(const Vec3& normal, float distFromOrigin);
	Plane3D(const Vec3& normal, const Vec3& randomPointOnPlane);

public:
	Vec3 m_normal;
	float m_distFromOrigin = 0.0f;
};