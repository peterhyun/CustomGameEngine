#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/Plane3D.hpp"
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include <vector>
#include <algorithm>

RaycastResult2D RaycastVsDisc2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& discCenter, float discRadius)
{
	RaycastResult2D raycastResult;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;
	raycastResult.m_rayFwdNormal = fwdNormal;

	Vec2 startToDiscCenter = discCenter - startPos;
	Vec2 fwdNormalRotated90Degrees = fwdNormal.GetRotated90Degrees();
	float startToDiscCenterProjectedToFwdNormal = DotProduct2D(startToDiscCenter, fwdNormal);
	float startToDiscCenterProjectedToFwdNormalRotated = DotProduct2D(startToDiscCenter, fwdNormalRotated90Degrees);
	if ((startToDiscCenterProjectedToFwdNormalRotated > discRadius) || (startToDiscCenterProjectedToFwdNormalRotated < -discRadius)) {
		return raycastResult;
	}
	if ((startToDiscCenterProjectedToFwdNormal < -discRadius) || (startToDiscCenterProjectedToFwdNormal > maxDist + discRadius)) {
		return raycastResult;
	}
	if (IsPointInsideDisc2D(startPos, discCenter, discRadius)) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f;
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactNormal = -fwdNormal;
		return raycastResult;
	}
	float discriminant = discRadius * discRadius - startToDiscCenterProjectedToFwdNormalRotated * startToDiscCenterProjectedToFwdNormalRotated;
	if (discriminant < 0.0f) {
		return raycastResult;
	}
	float impactDist = startToDiscCenterProjectedToFwdNormal - Sqrtf(discriminant);
	if ((impactDist > maxDist) || (impactDist < 0.0f))
		return raycastResult;
	raycastResult.m_didImpact = true;
	raycastResult.m_impactDist = impactDist;
	raycastResult.m_impactPos = startPos + (fwdNormal * impactDist);
	raycastResult.m_impactNormal = (raycastResult.m_impactPos - discCenter).GetNormalized();
	return raycastResult;
}

RaycastResult2D RaycastVsLineSegment2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& lineSegmentStart, const Vec2& lineSegmentEnd)
{
	RaycastResult2D raycastResult;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayMaxLength = maxDist;

	Vec2 leftNormal = fwdNormal.GetRotated90Degrees();
	Vec2 rayStartToLineStart = lineSegmentStart - startPos;
	Vec2 rayStartToLineEnd = lineSegmentEnd - startPos;
	Vec2 lineStartToLineEndDir = (lineSegmentEnd - lineSegmentStart).GetNormalized();
	float rayStartToLineStartDotLeftNormal = DotProduct2D(rayStartToLineStart, leftNormal);
	if (rayStartToLineStartDotLeftNormal == 0.0f) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = rayStartToLineStart.GetLength();
		raycastResult.m_impactNormal = lineStartToLineEndDir.GetRotated90Degrees();
		if (DotProduct2D(raycastResult.m_impactNormal, rayStartToLineStart) > 0.0f) {
			raycastResult.m_impactNormal *= -1.0f;
		}
		raycastResult.m_impactPos = lineSegmentStart;
	}
	float rayStartToLineEndDotLeftNormal = DotProduct2D(rayStartToLineEnd, leftNormal);
	if (rayStartToLineEndDotLeftNormal == 0.0f) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = rayStartToLineEnd.GetLength();
		raycastResult.m_impactNormal = lineStartToLineEndDir.GetRotated90Degrees();
		if (DotProduct2D(raycastResult.m_impactNormal, rayStartToLineEnd) > 0.0f) {
			raycastResult.m_impactNormal *= -1.0f;
		}
		raycastResult.m_impactPos = lineSegmentEnd;
	}
	//Now a straddle test
	if (rayStartToLineStartDotLeftNormal * rayStartToLineEndDotLeftNormal > 0.0f)
		return raycastResult;

	float t = -rayStartToLineStartDotLeftNormal / (rayStartToLineEndDotLeftNormal - rayStartToLineStartDotLeftNormal);
	raycastResult.m_impactPos = (1.0f - t)*lineSegmentStart + t*lineSegmentEnd;
	raycastResult.m_impactDist = DotProduct2D(raycastResult.m_impactPos - startPos, fwdNormal);
	if (raycastResult.m_impactDist > maxDist || raycastResult.m_impactDist < 0.0f) {
		return raycastResult;
	}
	raycastResult.m_didImpact = true;
	raycastResult.m_impactNormal = lineStartToLineEndDir.GetRotated90Degrees();
	if (DotProduct2D(raycastResult.m_impactNormal, fwdNormal) > 0.0f) {
		raycastResult.m_impactNormal *= -1.0f;
	}
	return raycastResult;
}

RaycastResult2D RaycastVsAABB2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& aabbMins, const Vec2& aabbMaxs)
{
	RaycastResult2D raycastResult;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayMaxLength = maxDist;
	//Edge cases
	if (IsPointInsideAABB2D(startPos, AABB2(aabbMins, aabbMaxs))) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactNormal = -fwdNormal;
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactDist = 0.0f;
		return raycastResult;
	}
	if (fwdNormal.x == 0.0f) {
		if (FloatRange(aabbMins.x, aabbMaxs.x).IsOnRange(startPos.x)) {
			if (fwdNormal.y > 0.0f) {
				if (FloatRange(startPos.y, startPos.y + maxDist).IsOverlappingWith(FloatRange(aabbMins.y, aabbMaxs.y))) {
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = (aabbMins.y - startPos.y) / fwdNormal.y;
					raycastResult.m_impactNormal = Vec2(0.0f, -1.0f);
					raycastResult.m_impactPos = startPos + fwdNormal * raycastResult.m_impactDist;
					return raycastResult;
				}
				return raycastResult;
			}
			else if (fwdNormal.y < 0.0f) {
				if (FloatRange(startPos.y - maxDist, startPos.y).IsOverlappingWith(FloatRange(aabbMins.y, aabbMaxs.y))) {
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = (aabbMaxs.y - startPos.y) / fwdNormal.y;
					raycastResult.m_impactNormal = Vec2(0.0f, 1.0f);
					raycastResult.m_impactPos = startPos + fwdNormal * raycastResult.m_impactDist;
					return raycastResult;
				}
				return raycastResult;
			}
		}
		return raycastResult;
	}
	if (fwdNormal.y == 0.0f) {
		if (FloatRange(aabbMins.y, aabbMaxs.y).IsOnRange(startPos.y)) {
			if (fwdNormal.x > 0.0f) {
				if (FloatRange(startPos.x, startPos.x + maxDist).IsOverlappingWith(FloatRange(aabbMins.x, aabbMaxs.x))) {
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = (aabbMins.x - startPos.x) / fwdNormal.x;
					raycastResult.m_impactNormal = Vec2(-1.0f, 0.0f);
					raycastResult.m_impactPos = startPos + fwdNormal * raycastResult.m_impactDist;
					return raycastResult;
				}
				return raycastResult;
			}
			else if (fwdNormal.x < 0.0f) {
				if (FloatRange(startPos.x - maxDist, startPos.x).IsOverlappingWith(FloatRange(aabbMins.x, aabbMaxs.x))) {
					raycastResult.m_didImpact = true;
					raycastResult.m_impactDist = (aabbMaxs.x - startPos.x) / fwdNormal.x;
					raycastResult.m_impactNormal = Vec2(1.0f, 0.0f);
					raycastResult.m_impactPos = startPos + fwdNormal * raycastResult.m_impactDist;
					return raycastResult;
				}
				return raycastResult;
			}
		}
		return raycastResult;
	}
	//General cases now
	float t_xMin = 0.0f;
	float t_xMax = 0.0f;
	float t_yMin = 0.0f;
	float t_yMax = 0.0f;
	if (fwdNormal.x > 0.0f) {
		t_xMin = (aabbMins.x - startPos.x) / fwdNormal.x;
		t_xMax = (aabbMaxs.x - startPos.x) / fwdNormal.x;
	}
	else {
		t_xMin = (aabbMaxs.x - startPos.x) / fwdNormal.x;
		t_xMax = (aabbMins.x - startPos.x) / fwdNormal.x;
	}
	if (fwdNormal.y > 0.0f) {
		t_yMin = (aabbMins.y - startPos.y) / fwdNormal.y;
		t_yMax = (aabbMaxs.y - startPos.y) / fwdNormal.y;
	}
	else {
		t_yMin = (aabbMaxs.y - startPos.y) / fwdNormal.y;
		t_yMax = (aabbMins.y - startPos.y) / fwdNormal.y;
	}

	FloatRange t_xRange(t_xMin, t_xMax);
	FloatRange t_yRange(t_yMin, t_yMax);
	FloatRange overlapping_t_range;
	if (t_xRange.GetOverlappingRange(t_yRange, overlapping_t_range)) {
		if (overlapping_t_range.m_min > maxDist || overlapping_t_range.m_min < 0.0f)
			return raycastResult;
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = overlapping_t_range.m_min;
		if (overlapping_t_range.m_min == t_xRange.m_min) {
			if (fwdNormal.x > 0.0f)
				raycastResult.m_impactNormal = Vec2(-1.0f, 0.0f);
			else
				raycastResult.m_impactNormal = Vec2(1.0f, 0.0f);
		}
		else if (overlapping_t_range.m_min == t_yRange.m_min) {
			if (fwdNormal.y > 0.0f)
				raycastResult.m_impactNormal = Vec2(0.0f, -1.0f);
			else
				raycastResult.m_impactNormal = Vec2(0.0f, 1.0f);
		}
		raycastResult.m_impactPos = startPos + raycastResult.m_impactDist * fwdNormal;
		return raycastResult;
	}
	else {
		return raycastResult;
	}
}

RaycastResult2D RaycastVsPlane2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Plane2D& plane)
{
	RaycastResult2D result;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayMaxLength = maxDist;
	result.m_rayStartPos = startPos;

	Vec2 endPos = startPos + fwdNormal * maxDist;
	float startPosAltitude = DotProduct2D(startPos, plane.m_normal) - plane.m_distFromOrigin;
	float endPosAltitude = DotProduct2D(endPos, plane.m_normal) - plane.m_distFromOrigin;
	if (startPosAltitude * endPosAltitude > 0.0f) {	//Not a hit
		return result;
	}
	else if (startPosAltitude == 0.0f) {
		result.m_didImpact = true;
		result.m_impactDist = 0.0f;
		result.m_impactNormal = plane.m_normal;
		result.m_impactPos = startPos;
		return result;
	}
	else if (endPosAltitude == 0.0f) {
		result.m_didImpact = true;
		result.m_impactDist = maxDist;
		result.m_impactNormal = plane.m_normal;
		result.m_impactPos = endPos;
		return result;
	}
	else {
		result.m_didImpact = true;
		float fwd_proj_planeN = DotProduct2D(fwdNormal, plane.m_normal);
		GUARANTEE_OR_DIE(fwd_proj_planeN != 0.0f, "This doesn't make sense");
		result.m_impactDist = -(startPosAltitude / fwd_proj_planeN);
		GUARANTEE_OR_DIE(result.m_impactDist > 0.0f, "Check signs again");
		result.m_impactNormal = plane.m_normal;
		result.m_impactPos = result.m_rayStartPos + result.m_rayFwdNormal * result.m_impactDist;
		return result;
	}
}

RaycastResult2D RaycastVsConvexPoly2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const ConvexPoly2D& convexPoly)
{
	RaycastResult2D raycastResult;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayMaxLength = maxDist;

	if (IsPointInsideConvexPoly2D(startPos, convexPoly)) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f;
		raycastResult.m_impactNormal = -fwdNormal;
		raycastResult.m_impactPos = startPos;
		return raycastResult;
	}

	//Raycast vs line segment for all edges
	const std::vector<Vec2>& points = convexPoly.m_points;
	unsigned int numPoints = (unsigned int)points.size();
	for (unsigned int i = 0; i < numPoints; i++) {
		RaycastResult2D tempResult = RaycastVsLineSegment2D(startPos, fwdNormal, maxDist, points[(i + 1) % numPoints], points[i]);
		if (tempResult.m_didImpact) {
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = tempResult.m_impactDist;
			raycastResult.m_impactNormal = tempResult.m_impactNormal;
			raycastResult.m_impactPos = tempResult.m_impactPos;
			return raycastResult;
		}
	}
	return raycastResult;
}

RaycastResult2D RaycastVsConvexHull2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const ConvexHull2D& convexHull, std::vector<RaycastResult2D>* out_entries, std::vector<RaycastResult2D>* out_exits)
{
	RaycastResult2D raycastResult;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayMaxLength = maxDist;

	if (IsPointInsideConvexHull2D(startPos, convexHull)) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f;
		raycastResult.m_impactNormal = -fwdNormal;
		raycastResult.m_impactPos = startPos;
		if (out_entries) {
			out_entries->clear();
		}
		if (out_exits) {
			out_exits->clear();
		}
		return raycastResult;
	}

	//Raycast vs plane2D for all edges
	//Check if it's entry or exit based on the dot product vs fwdNormal and plane normal
	const std::vector<Plane2D>& planes = convexHull.m_boundingPlanes;
	std::vector<RaycastResult2D> entries;
	std::vector<RaycastResult2D> exits;
	for (unsigned int i = 0; i < planes.size(); i++) {
		RaycastResult2D tempResult = RaycastVsPlane2D(startPos, fwdNormal, maxDist, planes[i]);
		if (tempResult.m_didImpact == false)
			continue;
		float dp = DotProduct2D(fwdNormal, planes[i].m_normal);
		if (dp < 0.0f) {
			entries.push_back(tempResult);
		}
		else if (dp > 0.0f) {
			exits.push_back(tempResult);
		}
		else {	//The line is just on top  of the plane
			return tempResult;
		}
	}

	if (out_entries) {
		*out_entries = entries;
	}
	if (out_exits) {
		*out_exits = exits;
	}

	//Edge cases 
	//If no entry...
	if (entries.size() == 0) {
		return raycastResult;
	}
	//If there are entries but no exit...
	if (exits.size() == 0) {
		//Get the last entry. Check if that is in the convex hull!!
		float lastEntryDist = entries[0].m_impactDist;
		int lastEntryIdx = 0;
		for (int i = 1; i < entries.size(); i++) {
			if (entries[i].m_impactDist > lastEntryDist) {
				lastEntryDist = entries[i].m_impactDist;
				lastEntryIdx = i;
			}
		}

		if (IsPointInsideConvexHull2D(entries[lastEntryIdx].m_impactPos, convexHull)) {
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = entries[lastEntryIdx].m_impactDist;
			raycastResult.m_impactNormal = entries[lastEntryIdx].m_impactNormal;
			raycastResult.m_impactPos = entries[lastEntryIdx].m_impactPos;
			return raycastResult;
		}
		else {	//Did NOT hit
			return raycastResult;
		}
	}
	
	//Now the general cases
	//Get the first exit
	float firstExitDist = exits[0].m_impactDist;
	int firstExitIdx = 0;
	for (int i = 1; i < exits.size(); i++) {
		if (exits[i].m_impactDist < firstExitDist) {
			firstExitDist = exits[i].m_impactDist;
			firstExitIdx = i;
		}
	}

	for (int i = 0; i < entries.size(); i++) {	//If there's an entry after the first exit
		if (entries[i].m_impactDist > firstExitDist) {	//Did NOT hit convex hull
			return raycastResult;
		}
	}

	//Get the last entry. That's the POSSIBLE collision point!
	float lastEntryDist = entries[0].m_impactDist;
	int lastEntryIdx = 0;
	for (int i = 1; i < entries.size(); i++) {
		if (entries[i].m_impactDist > lastEntryDist) {
			lastEntryDist = entries[i].m_impactDist;
			lastEntryIdx = i;
		}
	}

	//Check if the last entry is in the convex hull
	if (IsPointInsideConvexHull2D(entries[lastEntryIdx].m_impactPos, convexHull)) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = entries[lastEntryIdx].m_impactDist;
		raycastResult.m_impactNormal = entries[lastEntryIdx].m_impactNormal;
		raycastResult.m_impactPos = entries[lastEntryIdx].m_impactPos;
		return raycastResult;
	}
	else {
		return raycastResult;
	}
}

RaycastResult3D RaycastVsCylinderZ3D(const Vec3& startPos, const Vec3& fwdNormal, float maxDistance, const Vec2& cylinderCenterXY, float cylinderRadius, float cylinderMinZ, float cylinderMaxZ)
{
	RaycastResult3D raycastResult;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayMaxLength = maxDistance;

	FloatRange cylinderZFloatRange(cylinderMinZ, cylinderMaxZ);

	//Check if ray start is within the cylinder
	bool isStartPosXYInsideCylinderDisc2D = IsPointInsideDisc2D(Vec2(startPos), cylinderCenterXY, cylinderRadius);
	if (isStartPosXYInsideCylinderDisc2D && cylinderZFloatRange.IsOnRange(startPos.z)) {
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.0f;
		raycastResult.m_impactPos = startPos;
		raycastResult.m_impactNormal = -fwdNormal;
		return raycastResult;
	}

	//Edge case where ray is parallel to worldUp
	if (fwdNormal.x == 0.0f && fwdNormal.y == 0.0f) {
		if (isStartPosXYInsideCylinderDisc2D) {
			if (fwdNormal.z > 0.0f && startPos.z > cylinderMaxZ) {
				return raycastResult;
			}
			if (fwdNormal.z < 0.0f && startPos.z < cylinderMinZ) {
				return raycastResult;
			}
			float candidateImpactDist = (fwdNormal.z > 0.0f) ? cylinderMinZ - startPos.z : startPos.z - cylinderMaxZ;
			if (candidateImpactDist > maxDistance) {
				return raycastResult;
			}
			raycastResult.m_didImpact = true;
			raycastResult.m_impactDist = candidateImpactDist;
			raycastResult.m_impactPos = startPos + raycastResult.m_impactDist * fwdNormal;
			raycastResult.m_impactNormal = (fwdNormal.z > 0.0f) ? Vec3(0.0f, 0.0f, -1.0f) : Vec3(0.0f, 0.0f, 1.0f);
			return raycastResult;
		}
		return raycastResult;
	}

	Vec2 fromCylinderCenterToStartPosXY = Vec2(startPos) - cylinderCenterXY;
	float a = fwdNormal.x * fwdNormal.x + fwdNormal.y * fwdNormal.y;
	float b = fwdNormal.x * fromCylinderCenterToStartPosXY.x + fwdNormal.y * fromCylinderCenterToStartPosXY.y;
	float c = fromCylinderCenterToStartPosXY.x * fromCylinderCenterToStartPosXY.x + fromCylinderCenterToStartPosXY.y * fromCylinderCenterToStartPosXY.y - cylinderRadius * cylinderRadius;
	float determinant = b*b - a*c;

	if (determinant < 0.0f)
		return raycastResult;
	if (determinant == 0.0f) {
		float t = -b / a;
		if (t > maxDistance)
			return raycastResult;
		Vec3 impactPosInfiniteCylinder = startPos + t * fwdNormal;
		if (!cylinderZFloatRange.IsOnRange(impactPosInfiniteCylinder.z)) {
			return raycastResult;
		}
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = t;
		raycastResult.m_impactPos = impactPosInfiniteCylinder;
		raycastResult.m_impactNormal = Vec3(Vec2(impactPosInfiniteCylinder) - cylinderCenterXY, 0.0f);
		return raycastResult;
	}

	float t_1 = (-b + Sqrtf(determinant)) / a;
	float t_2 = (-b - Sqrtf(determinant)) / a;
	float t_minZ = (cylinderMinZ - startPos.z) / fwdNormal.z;
	float t_maxZ = (cylinderMaxZ - startPos.z) / fwdNormal.z;
	std::vector<float> validTVals;
	if (t_1 > 0.0f && t_1 < maxDistance && cylinderZFloatRange.IsOnRange((startPos + fwdNormal * t_1).z)) {
		validTVals.push_back(t_1);
	}
	if (t_2 > 0.0f && t_2 < maxDistance && cylinderZFloatRange.IsOnRange((startPos + fwdNormal * t_2).z)) {
		validTVals.push_back(t_2);
	}
	Vec3 rayImpact_t_minZ = startPos + t_minZ * fwdNormal;
	if (t_minZ > 0.0f && t_minZ < maxDistance && IsPointInsideDisc2D(Vec2(rayImpact_t_minZ), cylinderCenterXY, cylinderRadius)) {
		validTVals.push_back(t_minZ);
	}
	Vec3 rayImpact_t_maxZ = startPos + t_maxZ * fwdNormal;
	if (t_maxZ > 0.0f && t_maxZ < maxDistance && IsPointInsideDisc2D(Vec2(rayImpact_t_maxZ), cylinderCenterXY, cylinderRadius)) {
		validTVals.push_back(t_maxZ);
	}
	if (validTVals.size() == 0) {
		return raycastResult;
	}
	float t_smallestValid = *std::min_element(validTVals.begin(), validTVals.end());
	raycastResult.m_didImpact = true;
	raycastResult.m_impactDist = t_smallestValid;
	raycastResult.m_impactPos = startPos + fwdNormal * t_smallestValid;
	if (t_smallestValid == t_minZ) {
		raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
	}
	else if (t_smallestValid == t_maxZ) {
		raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
	}
	else {
		raycastResult.m_impactNormal = Vec3(Vec2(raycastResult.m_impactPos) - cylinderCenterXY, 0.0f).GetNormalized();
	}
	return raycastResult;
}

RaycastResult3D RaycastVsPlane3D(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const Plane3D& plane)
{
	RaycastResult3D result;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayMaxLength = maxDist;
	result.m_rayStartPos = startPos;

	Vec3 endPos = startPos + fwdNormal * maxDist;
	float startPosAltitude = DotProduct3D(startPos, plane.m_normal) - plane.m_distFromOrigin;
	float endPosAltitude = DotProduct3D(endPos, plane.m_normal) - plane.m_distFromOrigin;
	if (startPosAltitude * endPosAltitude > 0.0f) {	//Not a hit
		return result;
	}
	else if (startPosAltitude == 0.0f) {
		result.m_didImpact = true;
		result.m_impactDist = 0.0f;
		result.m_impactNormal = plane.m_normal;
		result.m_impactPos = startPos;
		return result;
	}
	else if (endPosAltitude == 0.0f) {
		result.m_didImpact = true;
		result.m_impactDist = maxDist;
		result.m_impactNormal = plane.m_normal;
		result.m_impactPos = endPos;
		return result;
	}
	else {
		result.m_didImpact = true;
		float fwd_proj_planeN = DotProduct3D(fwdNormal, plane.m_normal);
		GUARANTEE_OR_DIE(fwd_proj_planeN != 0.0f, "This doesn't make sense");
		result.m_impactDist = -(startPosAltitude / fwd_proj_planeN);
		GUARANTEE_OR_DIE(result.m_impactDist > 0.0f, "Check signs again");
		result.m_impactNormal = plane.m_normal;
		result.m_impactPos = result.m_rayStartPos + result.m_rayFwdNormal * result.m_impactDist;
		return result;
	}
}

Vec3 GetCursorRayDirectionWorldSpace(const Vec2& normalizedClientCursorPos, const Camera& worldCamera)
{
	if (worldCamera.GetCameraMode() != CameraMode::PERSPECTIVE) {
		ERROR_AND_DIE("GetCursorRayDirectionWorldSpace() is only for perspective cameras!");
	}

	float nearDist = worldCamera.GetPerspectiveNear();	
	float halfVerticalFOV = worldCamera.GetPerspectiveFOV() * 0.5f;
	float screenHeightWorld = 2.0f * nearDist * TanDegrees(halfVerticalFOV);
	float screenWidthWorld = screenHeightWorld * worldCamera.GetPerspectiveAspect();

	Vec2 screenOffsetWorld = Vec2(
		RangeMap(normalizedClientCursorPos.x, 0.0f, 1.0f, -0.5f * screenWidthWorld, 0.5f * screenWidthWorld), 
		RangeMap(normalizedClientCursorPos.y, 0.0f, 1.0f, -0.5f * screenHeightWorld, 0.5f * screenHeightWorld)
	);

	Vec3 cameraFwdWorld;
	Vec3 cameraLeftWorld;
	Vec3 cameraUpWorld;
	worldCamera.GetOrientation().GetAsVectors_XFwd_YLeft_ZUp(cameraFwdWorld, cameraLeftWorld, cameraUpWorld);

	Vec3 nearPlaneCenterWorldSpace = worldCamera.GetPosition() + cameraFwdWorld * nearDist;
	Vec3 finalMouseCursorPointWorldSpace = nearPlaneCenterWorldSpace + 
		(-cameraLeftWorld * screenOffsetWorld.x) + 
		(cameraUpWorld * screenOffsetWorld.y);

	return (finalMouseCursorPointWorldSpace - worldCamera.GetPosition()).GetNormalized();
}
