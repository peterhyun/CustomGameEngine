#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane2D.hpp"

ConvexHull2D::ConvexHull2D()
{
}

ConvexHull2D::ConvexHull2D(std::vector<Plane2D>& boundingPlanes) : m_boundingPlanes(boundingPlanes)
{
}

ConvexHull2D::ConvexHull2D(const ConvexPoly2D& convexPoly2D)
{
	const std::vector<Vec2>& points = convexPoly2D.m_points;
	unsigned int size = (unsigned int)points.size();
	for (unsigned int i = 0; i < size; i++) {
		Vec2 vecA2B = points[(i + 1) % size] - points[i];
		Vec2 planeNormal = vecA2B.GetNormalized().GetRotatedMinus90Degrees();
		float planeDistFromOrigin = DotProduct2D(points[i], planeNormal);
		m_boundingPlanes.emplace_back(planeNormal, planeDistFromOrigin);
	}
}