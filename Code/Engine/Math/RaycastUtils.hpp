#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <vector>

struct Plane2D;
struct Plane3D;
struct ConvexPoly2D;
struct ConvexHull2D;

struct RaycastResult2D
{
	bool m_didImpact = false;
	float m_impactDist = 0.f;
	Vec2 m_impactPos;
	Vec2 m_impactNormal;

	Vec2 m_rayFwdNormal;
	Vec2 m_rayStartPos;
	float m_rayMaxLength = 1.f;
};

struct RaycastResult3D
{
	bool m_didImpact = false;
	float m_impactDist = 0.f;
	Vec3 m_impactPos;
	Vec3 m_impactNormal;
	
	Vec3 m_rayFwdNormal;
	Vec3 m_rayStartPos;
	float m_rayMaxLength = 1.f;
};

RaycastResult2D RaycastVsDisc2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& discCenter, float discRadius);
RaycastResult2D RaycastVsLineSegment2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& lineSegmentStart, const Vec2& lineSegmentEnd);
RaycastResult2D RaycastVsAABB2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& aabbMins, const Vec2& aabbMaxs);
RaycastResult2D RaycastVsPlane2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Plane2D& plane);
RaycastResult2D RaycastVsConvexPoly2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const ConvexPoly2D& convexPoly);
RaycastResult2D RaycastVsConvexHull2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const ConvexHull2D& convexHull, std::vector<RaycastResult2D>* out_entries = nullptr, std::vector<RaycastResult2D>* out_exits = nullptr);

RaycastResult3D RaycastVsCylinderZ3D(const Vec3& start, const Vec3& direction, float distance, const Vec2& center, float radius, float minZ, float maxZ);
RaycastResult3D RaycastVsPlane3D(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const Plane3D& plane);

Vec3 GetCursorRayDirectionWorldSpace(const Vec2& normalizedClientCursorPos, const class Camera& worldCamera);