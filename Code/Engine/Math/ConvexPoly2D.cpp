#include "Engine/Math/ConvexPoly2D.hpp"

ConvexPoly2D::ConvexPoly2D()
{
}

ConvexPoly2D::ConvexPoly2D(std::vector<Vec2> ccwOrderedPoints) : m_points(ccwOrderedPoints)
{
}

void ConvexPoly2D::SetCenterPos(const Vec2& newCenterPos)
{
    Vec2 previousCenterPos = GetCenterPos();
    Vec2 fromPreviousToNewCenterPos = newCenterPos - previousCenterPos;
	for (Vec2& point : m_points) {
		point += fromPreviousToNewCenterPos;
	}
}

Vec2 ConvexPoly2D::GetCenterPos() const
{
    Vec2 centerPos;
    for (Vec2 point : m_points) {
        centerPos += point;
    }
    centerPos /= (float)m_points.size();
    return centerPos;
}
