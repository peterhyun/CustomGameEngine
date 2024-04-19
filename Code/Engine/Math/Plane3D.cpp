#include "Engine/Math/Plane3D.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane3D::Plane3D(const Vec3& normal, float distFromOrigin) : m_normal(normal), m_distFromOrigin(distFromOrigin)
{
}

Plane3D::Plane3D(const Vec3& normal, const Vec3& randomPointOnPlane) : m_normal(normal), m_distFromOrigin(DotProduct3D(normal, randomPointOnPlane))
{
}