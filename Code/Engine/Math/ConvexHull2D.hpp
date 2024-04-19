#pragma once
#include "Engine/Math/Plane2D.hpp"
#include <vector>

struct ConvexHull2D {
public:
	ConvexHull2D();
	ConvexHull2D(std::vector<Plane2D>& boundingPlanes);
	ConvexHull2D(const struct ConvexPoly2D& convexPoly2D);

public:
	std::vector<Plane2D> m_boundingPlanes;
};