#pragma once
#include "Engine/Math/Vec3.hpp"

struct InfiniteLine3 {
public:
	explicit InfiniteLine3(const Vec3& direction, const Vec3& point) : m_direction(direction), m_point(point) {};
	~InfiniteLine3() {};

public:
	Vec3 m_direction;
	Vec3 m_point;
};