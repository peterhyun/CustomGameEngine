#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB2::OBB2(const Vec2& center, const Vec2& iBasisNormal, const Vec2& halfDimensions)
: m_center(center), m_iBasisNormal(iBasisNormal), m_halfDimensions(halfDimensions), m_boundingSphereRadius(halfDimensions.GetLength())
{
}

void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	Vec2 jBasisNormal = m_iBasisNormal.GetRotated90Degrees();
	out_fourCornerWorldPositions[0] = m_center - m_halfDimensions.x * m_iBasisNormal + m_halfDimensions.y * jBasisNormal;
	out_fourCornerWorldPositions[1] = m_center - m_halfDimensions.x * m_iBasisNormal - m_halfDimensions.y * jBasisNormal;
	out_fourCornerWorldPositions[2] = m_center + m_halfDimensions.x * m_iBasisNormal - m_halfDimensions.y * jBasisNormal;
	out_fourCornerWorldPositions[3] = m_center + m_halfDimensions.x * m_iBasisNormal + m_halfDimensions.y * jBasisNormal;
}

Vec2 OBB2::GetLocalPosForWorldPos(const Vec2& worldPos) const
{
	Vec2 fromCenterToWorldPos = worldPos - m_center;
	return Vec2(DotProduct2D(fromCenterToWorldPos, m_iBasisNormal), DotProduct2D(fromCenterToWorldPos, m_iBasisNormal.GetRotated90Degrees()));
}

Vec2 OBB2::GetWorldPosForLocalPos(const Vec2& localPos) const
{
	Vec2 jBasisNormal = m_iBasisNormal.GetRotated90Degrees();
	return m_center + localPos.x * m_iBasisNormal + localPos.y * jBasisNormal;
}

void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_iBasisNormal.RotateDegrees(rotationDeltaDegrees);
}

void OBB2::SetHalfDimensions(const Vec2& halfDimensions)
{
	m_halfDimensions = halfDimensions;
	m_boundingSphereRadius = halfDimensions.GetLength();
}

void OBB2::SetIBasisNormal(const Vec2& iBasisNormal)
{
	m_iBasisNormal = iBasisNormal;
}

void OBB2::SetCenter(const Vec2& center)
{
	m_center = center;
}
