#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Plane3D.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/LineSegment3.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/InfiniteLine3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

float ConvertDegreesToRadians(float degrees) {
	return (degrees * (float)M_PI / 180.f);
}

float ConvertRadiansToDegrees(float radians) {
	return (radians * 180.f * (float)M_1_PI);
}

float CosDegrees(float degrees) {
	return cosf(ConvertDegreesToRadians(degrees));
}

float CosRadians(float radians)
{
	return cosf(radians);
}

float SinDegrees(float degrees) {
	return sinf(ConvertDegreesToRadians(degrees));
}

float SinRadians(float radians)
{
	return sinf(radians);
}

float TanDegrees(float degrees)
{
	return tanf(ConvertDegreesToRadians(degrees));
}

float AsinDegrees(float value)
{
	return ConvertRadiansToDegrees(asinf(value));
}

float Atan2Degrees(float y, float x) {
	return ConvertRadiansToDegrees(atan2f(y, x));
}

float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	float displacement = endDegrees - startDegrees;
	while (displacement > 180.0f) {
		displacement -= 360.0f;
	}
	while (displacement <= -180.0f) {
		displacement += 360.0f;
	}
	return displacement;
}

float GetShortestAngularDispRadians(float startRadians, float endRadians)
{
	float displacement = endRadians - startRadians;
	float twoTimesPi = float(2.0 * M_PI);
	while (displacement > M_PI) {
		displacement -= twoTimesPi;
	}
	while (displacement <= -M_PI) {
		displacement += twoTimesPi;
	}
	return displacement;
}

float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	float shortestAngularDispDegree = GetShortestAngularDispDegrees(currentDegrees, goalDegrees);
	if (shortestAngularDispDegree > 0) {
		if (maxDeltaDegrees > shortestAngularDispDegree) {
			return goalDegrees;
		}
		else {
			return currentDegrees + maxDeltaDegrees;
		}
	}
	else {
		if (maxDeltaDegrees > -shortestAngularDispDegree) {
			return goalDegrees;
		}
		else {
			return currentDegrees - maxDeltaDegrees;
		}
	}
}

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	return ConvertRadiansToDegrees(acosf(GetClamped(DotProduct2D(a, b) / (a.GetLength() * b.GetLength()), -1.0f, 1.0f)));
}

Vec2 RotatePointAroundAnotherPointDegrees2D(const Vec2& pointToRotate, const Vec2& fixedPoint, float degreesToRotate)
{
	Vec2 fixedPointToPointToRotate = pointToRotate - fixedPoint;
	return fixedPoint + fixedPointToPointToRotate.GetRotatedDegrees(degreesToRotate);
}

Vec2 ScalePointFromAnotherPoint2D(const Vec2& pointToScale, const Vec2& fixedPoint, float scale)
{
	Vec2 fixedPointToPointToScale = pointToScale - fixedPoint;
	return fixedPoint + fixedPointToPointToScale * scale;
}

float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x*b.x + a.y*b.y;
}

float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x * b.y - a.y * b.x;
}

Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	return Vec3(
		a.y * b.z - a.z * b.y,
		-a.x * b.z + a.z * b.x,
		a.x * b.y - a.y * b.x		
	);
}

float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB) {
	return (positionA - positionB).GetLength();
}

float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB) {
	return (positionA - positionB).GetLengthSquared();
}

float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB) {
	return (positionA - positionB).GetLength();
}

float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB) {
	return (positionA - positionB).GetLengthSquared();
}
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB) {
	return (positionA - positionB).GetLengthXY();
}
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB) {
	return (positionA - positionB).GetLengthXYSquared();
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return abs(pointB.x - pointA.x) + abs(pointB.y - pointA.y);
}

float GetVectorProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	return DotProduct2D(vectorToProject, normalizedVectorToProjectOnto);
}

Vec2 const GetVectorProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	float dotProduct2D = DotProduct2D(vectorToProject, normalizedVectorToProjectOnto);
	return dotProduct2D * normalizedVectorToProjectOnto;
}

float GetVectorProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	Vec3 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	return DotProduct3D(vectorToProject, normalizedVectorToProjectOnto);
}

Vec3 const GetVectorProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	Vec3 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	float dotProduct3D = DotProduct3D(vectorToProject, normalizedVectorToProjectOnto);
	return dotProduct3D * normalizedVectorToProjectOnto;
}

float GetDistanceBetweenLineSegmentAndPoint2D(const LineSegment2& lineSegment, const Vec2& point)
{
	Vec2 nearestPointOnLineSegment = GetNearestPointOnLineSegment2D(point, lineSegment);
	return (point - nearestPointOnLineSegment).GetLength();
}

float GetDistanceSquaredBetweenLineSegmentAndPoint2D(const LineSegment2& lineSegment, const Vec2& point)
{
	Vec2 nearestPointOnLineSegment = GetNearestPointOnLineSegment2D(point, lineSegment);
	return (point - nearestPointOnLineSegment).GetLengthSquared();
}

float GetDistanceBetweenLineSegmentAndPoint3D(const LineSegment3& lineSegment, const Vec3& point)
{
	Vec3 nearestPointOnLineSegment = GetNearestPointOnLineSegment3D(point, lineSegment);
	return (point - nearestPointOnLineSegment).GetLength();
}

float GetDistanceSquaredBetweenLineSegmentAndPoint3D(const LineSegment3& lineSegment, const Vec3& point)
{
	Vec3 nearestPointOnLineSegment = GetNearestPointOnLineSegment3D(point, lineSegment);
	return (point - nearestPointOnLineSegment).GetLengthSquared();
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	return (point - discCenter).GetLengthSquared() <= discRadius * discRadius;
}

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	return box.IsPointInside(point);
}

bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule)
{
	return IsPointInsideCapsule2D(point, capsule.GetStartPos(), capsule.GetEndPos(), capsule.GetRadius());
}

bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 closestPointOnLineSegment = GetNearestPointOnLineSegment2D(point, boneStart, boneEnd);
	return ((point - closestPointOnLineSegment).GetLengthSquared() <= radius * radius);
}

bool IsPointInsideCapsule3D(Vec3 const& point, Capsule3 const& capsule)
{
	Vec3 closestPointOnLineSegment = GetNearestPointOnLineSegment3D(point, capsule.GetStartPos(), capsule.GetEndPos());
	float radius = capsule.GetRadius();
	return ((point - closestPointOnLineSegment).GetLengthSquared() <= radius * radius);
}

bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox)
{
	Vec2 obb2LocalCoords = orientedBox.GetLocalPosForWorldPos(point);
	if (fabsf(obb2LocalCoords.x) >= orientedBox.m_halfDimensions.x) {
		return false;
	}
	if (fabsf(obb2LocalCoords.y) >= orientedBox.m_halfDimensions.y) {
		return false;
	}
	return true;
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (IsPointInsideDisc2D(point, sectorTip, sectorRadius)) {
		Vec2 fromSectorTipToPoint = point - sectorTip;
		float fromSectorTipToPointDegrees = fromSectorTipToPoint.GetOrientationDegrees();
		float shortestAngularDisplacement = GetShortestAngularDispDegrees(fromSectorTipToPointDegrees, sectorForwardDegrees);
		return fabsf(shortestAngularDisplacement) < (sectorApertureDegrees * 0.5f);
	}
	else
		return false;
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (IsPointInsideDisc2D(point, sectorTip, sectorRadius)) {
		Vec2 fromSectorTipToPoint = point - sectorTip;
		//Get the angle between two vectors
		float degrees = GetAngleDegreesBetweenVectors2D(fromSectorTipToPoint, sectorForwardNormal);
		return degrees <= (0.5f * sectorApertureDegrees);
	}
	else
		return false;
}

bool IsPointInsideSphere(const Vec3& point, const Vec3& sphereCenter, float sphereRadius)
{
	return (point - sphereCenter).GetLengthSquared() <= sphereRadius * sphereRadius;
}

bool IsPointInsideConvexPoly2D(const Vec2& point, const ConvexPoly2D& convexPoly)
{
	const std::vector<Vec2>& points = convexPoly.m_points;
	unsigned int size = (unsigned int)points.size();
	for (unsigned int i = 0; i < size; i++) {
		Vec2 vectorA = (points[(i + 1) % size] - points[i]);
		Vec2 vectorB = (point - points[i]);
		float crossProductResult = CrossProduct2D(vectorA, vectorB);
		if (crossProductResult < 0.0f) {
			return false;
		}
	}
	return true;
}

bool IsPointInsideConvexHull2D(const Vec2& point, const ConvexHull2D& convexHull)
{
	float epsilon = 0.0001f;
	const std::vector<Plane2D>& planes = convexHull.m_boundingPlanes;
	unsigned int size = (unsigned int)planes.size();
	for (unsigned int i = 0; i < size; i++) {
		float dotProductResult = DotProduct2D(planes[i].m_normal, point);
		if (dotProductResult - planes[i].m_distFromOrigin > epsilon) {
			return false;
		}
	}
	return true;
}

bool DoAABB2sOverlap(const AABB2& first, const AABB2& second)
{  
	// Check for separation in x axis
	bool xOverlap = first.m_maxs.x >= second.m_mins.x && second.m_maxs.x >= first.m_mins.x;

	// Check for separation in y axis
	bool yOverlap = first.m_maxs.y >= second.m_mins.y && second.m_maxs.y >= first.m_mins.y;

	// If there is no separation along both x and y axes, the AABBs overlap
	return xOverlap && yOverlap;
}

bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB) {
	float distanceSquared = (centerA - centerB).GetLengthSquared();
	float radiusSum = radiusA + radiusB;
	return distanceSquared <= (radiusSum * radiusSum);
}

bool DoDiscAndAABB2DOverlap(Vec2 const& discCenter, float discRadius, const AABB2& aabb2)
{
	Vec2 nearestPointToDiscOnAABB2 = aabb2.GetNearestPoint(discCenter);
	return IsPointInsideDisc2D(nearestPointToDiscOnAABB2, discCenter, discRadius);
}

bool DoDiscAndOBB2Overlap(Vec2 const& discCenter, float discRadius, const OBB2& obb2)
{
	//Do Disc/Disc overlap check first
	if (!DoDiscsOverlap(discCenter, discRadius, obb2.m_center, obb2.m_boundingSphereRadius)) {
		return false;
	}
	Vec2 nearestPointOnOBB2 = GetNearestPointOnOBB2D(discCenter, obb2);
	if ((nearestPointOnOBB2 - discCenter).GetLengthSquared() <= discRadius * discRadius) {
		return true;
	}
	return false;
}

bool DoDiscAndCapsule2Overlap(Vec2 const& discCenter, float discRadius, const Capsule2& capsule2)
{
	//Do Disc/Disc overlap check first
	if (!DoDiscsOverlap(discCenter, discRadius, capsule2.GetBoundingSphereCenter(), capsule2.GetBoundingSphereRadius())) {
		return false;
	}
	Vec2 nearestPointOnCapsule2 = GetNearestPointOnCapsule2D(discCenter, capsule2);
	if ((nearestPointOnCapsule2 - discCenter).GetLengthSquared() <= discRadius * discRadius) {
		return true;
	}
	return false;
}

bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB) {
	float distance = (centerA - centerB).GetLength();
	return distance < (radiusA + radiusB);
}

bool DoZCylindersOverlap(Vec3 const& bottomCenterA, float heightA, float radiusA, Vec3 const& bottomCenterB, float heightB, float radiusB)
{
	FloatRange cylinderA_ZRange(bottomCenterA.z, bottomCenterA.z + heightA);
	FloatRange cylinderB_ZRange(bottomCenterB.z, bottomCenterB.z + heightB);
	if (!cylinderA_ZRange.IsOverlappingWith(cylinderB_ZRange))
		return false;
	return DoDiscsOverlap(Vec2(bottomCenterA), radiusA, Vec2(bottomCenterB), radiusB);
}

bool DoZCylinderAndAABB3DOverlap(Vec3 const& cylinderBottomCenter, float cylinderRadius, const FloatRange& cylinderZRange, const AABB3& aabb3)
{
	FloatRange aabb3ZRange(aabb3.m_mins.z, aabb3.m_maxs.z);
	if (!cylinderZRange.IsOverlappingWith(aabb3ZRange))
		return false;
	return DoDiscAndAABB2DOverlap(Vec2(cylinderBottomCenter), cylinderRadius, aabb3.GetBoundsXY());
}

bool DoAABB3AndSphereOverlap(const AABB3& aabb, const Vec3& sphereCenter, float sphereRadius)
{
	Vec3 nearestPointToSphereOnAABB3 = aabb.GetNearestPoint(sphereCenter);
	return IsPointInsideSphere(nearestPointToSphereOnAABB3, sphereCenter, sphereRadius);
}

Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	if (IsPointInsideDisc2D(referencePosition, discCenter, discRadius)) {
		return referencePosition;
	}
	else {
		Vec2 fromCenterToReferenceDir = (referencePosition - discCenter).GetNormalized();
		return discCenter + fromCenterToReferenceDir * discRadius;
	}
}

Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2& box)
{
	return box.GetNearestPoint(referencePos);
}

Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine)
{
	return GetNearestPointOnInfiniteLine2D(referencePos, infiniteLine.m_start, infiniteLine.m_end);
}

Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 lineVector = anotherPointOnLine - pointOnLine;
	Vec2 fromAnyLinePointToReference = referencePos - pointOnLine;
	Vec2 fromStartToReferenceProjected = GetVectorProjectedOnto2D(fromAnyLinePointToReference, lineVector);
	return pointOnLine + fromStartToReferenceProjected;
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& lineSegment)
{
	return GetNearestPointOnLineSegment2D(referencePos, lineSegment.m_start, lineSegment.m_end);
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd)
{
	Vec2 lineSegmentVector = lineSegEnd - lineSegStart;
	Vec2 fromStartToReferencePos = referencePos - lineSegStart;
	Vec2 fromEndToReferencePos = referencePos - lineSegEnd;
	if (DotProduct2D(lineSegmentVector, fromStartToReferencePos) < 0.0f) {
		return lineSegStart;
	}
	if (DotProduct2D(lineSegmentVector, fromEndToReferencePos) > 0.0f) {
		return lineSegEnd;
	}
	Vec2 fromStartToReferenceProjected = GetVectorProjectedOnto2D(fromStartToReferencePos, lineSegmentVector);
	return lineSegStart + fromStartToReferenceProjected;
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 const& capsule)
{
	Vec2 nearestOnBone = GetNearestPointOnLineSegment2D(referencePos, capsule.GetLineSegment());
	Vec2 fromClosestOnBoneToReferencePos = referencePos - nearestOnBone;
	fromClosestOnBoneToReferencePos.ClampLength(capsule.GetRadius());
	return nearestOnBone + fromClosestOnBoneToReferencePos;
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 nearestOnBone = GetNearestPointOnLineSegment2D(referencePos, boneStart, boneEnd);
	Vec2 fromClosestOnBoneToReferencePos = referencePos - nearestOnBone;
	fromClosestOnBoneToReferencePos.ClampLength(radius);
	return nearestOnBone + fromClosestOnBoneToReferencePos;
}

Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox)
{
	Vec2 obb2LocalCoords = orientedBox.GetLocalPosForWorldPos(referencePos);
	float clampedLocalCoord_i = GetClamped(obb2LocalCoords.x, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x);
	float clampedLocalCoord_j = GetClamped(obb2LocalCoords.y, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y);
	return orientedBox.m_center + clampedLocalCoord_i * orientedBox.m_iBasisNormal + clampedLocalCoord_j * orientedBox.m_iBasisNormal.GetRotated90Degrees();
}

Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePos, LineSegment3 const& lineSegment)
{
	const Vec3& lineSegStart = lineSegment.m_start;
	const Vec3& lineSegEnd = lineSegment.m_end;
	Vec3 lineSegmentVector = lineSegEnd - lineSegStart;
	Vec3 fromStartToReferencePos = referencePos - lineSegStart;
	Vec3 fromEndToReferencePos = referencePos - lineSegEnd;
	if (DotProduct3D(lineSegmentVector, fromStartToReferencePos) < 0.0f) {
		return lineSegStart;
	}
	if (DotProduct3D(lineSegmentVector, fromEndToReferencePos) > 0.0f) {
		return lineSegEnd;
	}
	Vec3 fromStartToReferenceProjected = GetVectorProjectedOnto3D(fromStartToReferencePos, lineSegmentVector);
	return lineSegStart + fromStartToReferenceProjected;
}

Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePos, Vec3 const& lineSegStart, Vec3 const& lineSegEnd)
{
	Vec3 lineSegmentVector = lineSegEnd - lineSegStart;
	Vec3 fromStartToReferencePos = referencePos - lineSegStart;
	Vec3 fromEndToReferencePos = referencePos - lineSegEnd;
	if (DotProduct3D(lineSegmentVector, fromStartToReferencePos) < 0.0f) {
		return lineSegStart;
	}
	if (DotProduct3D(lineSegmentVector, fromEndToReferencePos) > 0.0f) {
		return lineSegEnd;
	}
	Vec3 fromStartToReferenceProjected = GetVectorProjectedOnto3D(fromStartToReferencePos, lineSegmentVector);
	return lineSegStart + fromStartToReferenceProjected;
}

bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	if (IsPointInsideDisc2D(fixedPoint, mobileDiscCenter, discRadius)) {
		float fixedPointAndCenterDistance = (mobileDiscCenter - fixedPoint).GetLength();
		Vec2 fromFixedPointToCenterDir = (mobileDiscCenter - fixedPoint).GetNormalized();
		mobileDiscCenter = mobileDiscCenter + ((discRadius - fixedPointAndCenterDistance) * fromFixedPointToCenterDir);
		return true;
	}
	else {
		return false;
	}
}

bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	if (DoDiscsOverlap(mobileDiscCenter, mobileDiscRadius, fixedDiscCenter, fixedDiscRadius)) {
		float distanceBetweenCenters = (mobileDiscCenter - fixedDiscCenter).GetLength();
		float distanceToMove = mobileDiscRadius + fixedDiscRadius - distanceBetweenCenters;
		Vec2 fromFixedCenterToMobileCenterDir = (mobileDiscCenter - fixedDiscCenter).GetNormalized();
		mobileDiscCenter = mobileDiscCenter + (distanceToMove * fromFixedCenterToMobileCenterDir);
		return true;
	}
	else {
		return false;
	}
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	if (DoDiscsOverlap(aCenter, aRadius, bCenter, bRadius)) {
		float distanceBetweenCenters = (aCenter - bCenter).GetLength();
		float distanceToMove = aRadius + bRadius - distanceBetweenCenters;
		Vec2 fromBCenterToACenterDir = (aCenter - bCenter).GetNormalized();
		aCenter = aCenter + (0.5f * distanceToMove * fromBCenterToACenterDir);
		bCenter = bCenter - (0.5f * distanceToMove * fromBCenterToACenterDir);
		return true;
	}
	else {
		return false;
	}
}

bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	Vec2 nearestPointOnFixedBox = fixedBox.GetNearestPoint(mobileDiscCenter);
	return PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPointOnFixedBox);
}

bool PushDiscOutOfFixedOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& fixedOBB2)
{
	Vec2 nearestPointOnFixedOBB2 = GetNearestPointOnOBB2D(mobileDiscCenter, fixedOBB2);
	return PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPointOnFixedOBB2);
}

bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& fixedCapsule)
{
	Vec2 nearestPointOnFixedCapsule2 = GetNearestPointOnCapsule2D(mobileDiscCenter, fixedCapsule);
	return PushDiscOutOfFixedPoint2D(mobileDiscCenter, discRadius, nearestPointOnFixedCapsule2);
}

bool BounceDiscsOffEachOther2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA, float elasticityB)
{
	if (DoDiscsOverlap(posA, radiusA, posB, radiusB)) {
		PushDiscsOutOfEachOther2D(posA, radiusA, posB, radiusB);
		float elasticity = elasticityA * elasticityB;
		Vec2 fromAToBDir = (posB - posA).GetNormalized();
		Vec2 velocityA_NormalDir = DotProduct2D(fromAToBDir, velocityA) * fromAToBDir;
		Vec2 velocityA_TangentDir = velocityA - velocityA_NormalDir;

		Vec2 velocityB_NormalDir = DotProduct2D(fromAToBDir, velocityB) * fromAToBDir;
		Vec2 velocityB_TangentDir = velocityB - velocityB_NormalDir;

		//Only swap when it's diverging from each other
		Vec2 relativeVelocityOfBToA_NormalDir = velocityB_NormalDir - velocityA_NormalDir;
		if (DotProduct2D(relativeVelocityOfBToA_NormalDir, fromAToBDir) < 0.0f) {	//They're converging
			velocityA = velocityA_TangentDir + velocityB_NormalDir * elasticity;
			velocityB = velocityB_TangentDir + velocityA_NormalDir * elasticity;
		}
		else {	//They're diverging -> Don't swap normal velocity
			velocityA = velocityA_TangentDir + velocityA_NormalDir * elasticity;
			velocityB = velocityB_TangentDir + velocityB_NormalDir * elasticity;
		}
		return true;
	}
	else {
		return false;
	}
}

bool BounceDiscOffOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, const Vec2& fixedDiscCenter, float fixedDiscRadius, float mobileDiscElasticity, float fixedDiscElasticity)
{
	if (DoDiscsOverlap(mobileDiscCenter, mobileDiscRadius, fixedDiscCenter, fixedDiscRadius)) {
		PushDiscOutOfFixedDisc2D(mobileDiscCenter, mobileDiscRadius, fixedDiscCenter, fixedDiscRadius);
		float elasticity = mobileDiscElasticity * fixedDiscElasticity;
		Vec2 fromMobileToFixedDir = (fixedDiscCenter - mobileDiscCenter).GetNormalized();
		Vec2 velocityMobileDiscNormalDir = DotProduct2D(fromMobileToFixedDir, mobileDiscVelocity) * fromMobileToFixedDir;
		Vec2 velocityMobileDiscTangentDir = mobileDiscVelocity - velocityMobileDiscNormalDir;
		//Only swap when it's diverging from each other
		Vec2 fromFixedToMobileDisc = mobileDiscCenter - fixedDiscCenter;
		if (DotProduct2D(velocityMobileDiscNormalDir, fromFixedToMobileDisc) < 0.0f) {
			mobileDiscVelocity = velocityMobileDiscTangentDir + (-velocityMobileDiscNormalDir * elasticity);
		}
		else {
			mobileDiscVelocity = velocityMobileDiscTangentDir + (velocityMobileDiscNormalDir * elasticity);
		}
		return true;
	}
	else {
		return false;
	}
}

bool BounceDiscOffOfFixedOBB2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, const OBB2& fixedOBB2, float mobileDiscElasticity, float fixedOBB2Elasticity)
{
	if (DoDiscAndOBB2Overlap(mobileDiscCenter, mobileDiscRadius, fixedOBB2)) {
		PushDiscOutOfFixedOBB2D(mobileDiscCenter, mobileDiscRadius, fixedOBB2);
		float elasticity = mobileDiscElasticity * fixedOBB2Elasticity;
		Vec2 closestPointOnOBB2 = GetNearestPointOnOBB2D(mobileDiscCenter, fixedOBB2);
		Vec2 normalVector = (mobileDiscCenter - closestPointOnOBB2).GetNormalized();
		Vec2 velocityMobileDiscNormalDir = DotProduct2D(normalVector, mobileDiscVelocity) * normalVector;
		Vec2 velocityMobileDiscTangentDir = mobileDiscVelocity - velocityMobileDiscNormalDir;
		if (DotProduct2D(velocityMobileDiscNormalDir, normalVector) < 0.0f) {
			mobileDiscVelocity = velocityMobileDiscTangentDir + (-velocityMobileDiscNormalDir * elasticity);
		}
		else {
			mobileDiscVelocity = velocityMobileDiscTangentDir + (velocityMobileDiscNormalDir * elasticity);
		}
		return true;
	}
	else {
		return false;
	}
}

bool BounceDiscOffOfFixedCapsule2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, const Capsule2& fixedCapsule, float mobileDiscElasticity, float fixedCapsuleElasticity)
{
	if (DoDiscAndCapsule2Overlap(mobileDiscCenter, mobileDiscRadius, fixedCapsule)) {
		PushDiscOutOfFixedCapsule2D(mobileDiscCenter, mobileDiscRadius, fixedCapsule);
		float elasticity = mobileDiscElasticity * fixedCapsuleElasticity;
		Vec2 closestPointOnCapsule = GetNearestPointOnCapsule2D(mobileDiscCenter, fixedCapsule);
		Vec2 normalVector = (mobileDiscCenter - closestPointOnCapsule).GetNormalized();
		Vec2 velocityMobileDiscNormalDir = DotProduct2D(normalVector, mobileDiscVelocity) * normalVector;
		Vec2 velocityMobileDiscTangentDir = mobileDiscVelocity - velocityMobileDiscNormalDir;
		if (DotProduct2D(velocityMobileDiscNormalDir, normalVector) < 0.0f) {
			mobileDiscVelocity = velocityMobileDiscTangentDir + (-velocityMobileDiscNormalDir * elasticity);
		}
		else {
			mobileDiscVelocity = velocityMobileDiscTangentDir + (velocityMobileDiscNormalDir * elasticity);
		}
		return true;
	}
	else {
		return false;
	}
}

void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation) {
	posToTransform *= uniformScale;
	posToTransform.RotateDegrees(rotationDegrees);
	posToTransform += translation;
}

void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	posToTransform = iBasis * posToTransform.x + jBasis * posToTransform.y + translation;
}

void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY) {
	posToTransform.x *= scaleXY;
	posToTransform.y *= scaleXY;
	posToTransform = posToTransform.GetRotatedAboutZDegrees(zRotationDegrees);
	posToTransform += Vec3(translationXY.x, translationXY.y, 0);
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	Vec2 posToTransformXY = Vec2(posToTransform.x, posToTransform.y);
	TransformPosition2D(posToTransformXY, iBasis, jBasis, translation);
	posToTransform = Vec3(posToTransformXY, posToTransform.z);
}

Mat44 GetBillboardMatrix(BillboardType billboardType, const Mat44& targetMatrix, const Vec3& billboardPosition, const Vec2& billboardScale)
{
	Vec3 targetStart = targetMatrix.GetTranslation3D();
	Vec3 targetFwd = targetMatrix.GetIBasis3D();
	Vec3 targetLeft = targetMatrix.GetJBasis3D();
	Vec3 targetUp = targetMatrix.GetKBasis3D();
	Vec3 worldFwd = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 worldLeft = Vec3(0.0f, 1.0f, 0.0f);
	Vec3 worldUp = Vec3(0.0f, 0.0f, 1.0f);
	Mat44 billboardMatrix;
	Vec3 billboardFwd;
	Vec3 billboardLeft;
	Vec3 billboardUp;
	switch (billboardType) {
	case BillboardType::FULL_CAMERA_OPPOSING:
		billboardFwd = -targetFwd;
		billboardLeft = -targetLeft;
		billboardUp = targetUp;
		break;
	case BillboardType::FULL_CAMERA_FACING:
		billboardFwd = (targetStart - billboardPosition).GetNormalized();
		if (fabsf(DotProduct3D(billboardFwd, worldUp) < 1.0f)) {
			billboardLeft = CrossProduct3D(worldUp, billboardFwd).GetNormalized();
			billboardUp = CrossProduct3D(billboardFwd, billboardLeft).GetNormalized();
		}
		else {
			billboardUp = CrossProduct3D(billboardFwd, worldLeft).GetNormalized();
			billboardLeft = CrossProduct3D(billboardUp, billboardFwd).GetNormalized();
		}
		break;
	case BillboardType::WORLD_UP_CAMERA_FACING: {
		Vec3 fromBillboardToTarget = (targetStart - billboardPosition);
		if (fromBillboardToTarget.x != 0.0f || fromBillboardToTarget.y != 0.0f) {
			billboardFwd = Vec3(fromBillboardToTarget.x, fromBillboardToTarget.y, 0.0f).GetNormalized();
			billboardUp = worldUp;
			billboardLeft = CrossProduct3D(worldUp, billboardFwd).GetNormalized();
		}
		else {
			billboardFwd = worldFwd;
			billboardUp = worldUp;
			billboardLeft = worldLeft;
		}
		break;
	}
	case BillboardType::WORLD_UP_CAMERA_OPPOSING:
		billboardFwd = Vec3(-targetFwd.x, -targetFwd.y, 0.0f).GetNormalized();
		billboardUp = worldUp;
		billboardLeft = CrossProduct3D(billboardUp, billboardFwd).GetNormalized();
		break;
	default:
		ERROR_RECOVERABLE("BillboardType must be set correctly when calling GetBillboardMatrix()");
	}
	billboardMatrix.SetTranslation3D(billboardPosition);
	billboardMatrix.SetIJK3D(billboardFwd, billboardLeft * billboardScale.x, billboardUp * billboardScale.y);
	return billboardMatrix;
}

float NormalizeByte(unsigned char byteValue)
{
	return RangeMapClamped((float)byteValue, 0.0f, 255.0f, 0.0f, 1.0f);
}

unsigned char DenormalizeByte(float zeroToOne)
{
	if (zeroToOne == 1.0f) {
		return 255;
	}
	return (unsigned char)RangeMapClamped((float)zeroToOne, 0.0f, 1.0f, 0.0f, 256.0f);
}

float ComputeCubicBezier1D(float A, float B, float C, float D, float t)
{
	float one_minus_t = 1.0f - t;
	return one_minus_t * one_minus_t * one_minus_t * A +
		3.0f * one_minus_t * one_minus_t * t * B+
		3.0f * one_minus_t * t * t * C+
		t * t * t * D;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	float one_minus_t = 1.0f - t;
	return one_minus_t * one_minus_t * one_minus_t * one_minus_t * one_minus_t * A +
		5.0f * one_minus_t * one_minus_t * one_minus_t * one_minus_t * t * B +
		10.0f * one_minus_t * one_minus_t * one_minus_t * t * t * C +
		10.0f * one_minus_t * one_minus_t * t * t * t * D +
		5.0f * one_minus_t * t * t * t * t * E +
		t * t * t * t * t * F;
}

float ComputeCubicBezierTangent1D(float A, float B, float C, float D, float t)
{
	float one_minus_t = 1.0f - t;
	float t_square = t * t;
	float one_minus_t_square = one_minus_t * one_minus_t;
	return -3.0f * one_minus_t_square * A +
		(3.0f * one_minus_t_square - 6.0f * t * one_minus_t) * B +
		(-3 * t_square + 6 * t * one_minus_t )* C +
		3 * t_square * D;
}

float SmoothStart2(float input)
{
	return input * input;
}

float SmoothStart3(float input)
{
	return input * input * input;
}

float SmoothStart4(float input)
{
	return input * input * input * input;
}

float SmoothStart5(float input)
{
	return input * input * input * input * input;
}

float SmoothStart6(float input)
{
	return input * input * input * input * input * input;
}

float SmoothStop2(float input)
{
	float one_minus_input = 1.0f - input;
	return 1.0f - one_minus_input * one_minus_input;
}

float SmoothStop3(float input)
{
	float one_minus_input = 1.0f - input;
	return 1.0f - one_minus_input * one_minus_input * one_minus_input;
}

float SmoothStop4(float input)
{
	float one_minus_input = 1.0f - input;
	return 1.0f - one_minus_input * one_minus_input * one_minus_input * one_minus_input;
}

float SmoothStop5(float input)
{
	float one_minus_input = 1.0f - input;
	return 1.0f - one_minus_input * one_minus_input * one_minus_input * one_minus_input * one_minus_input;
}

float SmoothStop6(float input)
{
	float one_minus_input = 1.0f - input;
	return 1.0f - one_minus_input * one_minus_input * one_minus_input * one_minus_input * one_minus_input * one_minus_input;
}

float SmoothStep3(float input)
{
	return 3.0f * input * input - 2 * input * input * input;
}

float SmoothStep5(float input)
{
	return 6.0f * input * input * input * input * input - 15.0f * input * input * input * input + 10.0f * input * input * input;
}

float Hesitate3(float input)
{
	float one_minus_input = 1.0f - input;
	return 3.0f * input * (one_minus_input) * (one_minus_input) + input * input * input;
}

float Hesitate5(float input)
{
	float one_minus_input = 1.0f - input;
	return 5.0f * input * (one_minus_input) * (one_minus_input) * (one_minus_input) * (one_minus_input) +
		10.0f * input * input * input * (one_minus_input) * (one_minus_input) +
		input * input * input * input * input;
}

float CustomFunkyEasingFunction(float input)
{
	float oneMinusInput = 1.0f - input;
	//float b03 = GetCombination(3, 0) * oneMinusInput * oneMinusInput * oneMinusInput;
	float b15 = GetCombination(5, 1) * oneMinusInput * oneMinusInput * oneMinusInput * oneMinusInput * input;
	float b25 = GetCombination(5, 2) * oneMinusInput * oneMinusInput * oneMinusInput * input * input;
	float b35 = GetCombination(5, 3) * oneMinusInput * oneMinusInput * input * input * input;
	float b45 = GetCombination(5, 4) * oneMinusInput* input * input * input * input;
	float b55 = GetCombination(5, 5) * input * input * input * input * input;
	return 1.2f * b15 * input + 0.5f * b25 * (1.0f - input) + b35 * 1.5f * input + b45 * (1.0f - input) + b55 * input * input;
}

unsigned int GetFactorial(unsigned int n)
{
	unsigned int result = 1;
	for (unsigned int i = 2; i <= n ; i++) {
		result *= i;
	}
	return result;
}

AABB2 MergeAABB2s(const AABB2& firstAABB, const AABB2& secondAABB)
{
	Vec2 newMin;
	Vec2 newMax;
	if (firstAABB.m_mins.x < secondAABB.m_mins.x) {
		newMin.x = firstAABB.m_mins.x;
	}
	else {
		newMin.x = secondAABB.m_mins.x;
	}
	if (firstAABB.m_mins.y < secondAABB.m_mins.y) {
		newMin.y = firstAABB.m_mins.y;
	}
	else {
		newMin.y = secondAABB.m_mins.y;
	}

	if (firstAABB.m_maxs.x < secondAABB.m_maxs.x) {
		newMax.x = secondAABB.m_maxs.x;
	}
	else {
		newMax.x = firstAABB.m_maxs.x;
	}
	if (firstAABB.m_maxs.y < secondAABB.m_maxs.y) {
		newMax.y = secondAABB.m_maxs.y;
	}
	else {
		newMax.y = firstAABB.m_maxs.y;
	}
	return AABB2(newMin, newMax);
}

AABB3 MergeAABB3s(const AABB3& firstAABB, const AABB3& secondAABB)
{
	Vec3 newMin;
	Vec3 newMax;
	if (firstAABB.m_mins.x < secondAABB.m_mins.x) {
		newMin.x = firstAABB.m_mins.x;
	}
	else {
		newMin.x = secondAABB.m_mins.x;
	}
	if (firstAABB.m_mins.y < secondAABB.m_mins.y) {
		newMin.y = firstAABB.m_mins.y;
	}
	else {
		newMin.y = secondAABB.m_mins.y;
	}
	if (firstAABB.m_mins.z < secondAABB.m_mins.z) {
		newMin.z = firstAABB.m_mins.z;
	}
	else {
		newMin.z = secondAABB.m_mins.z;
	}

	if (firstAABB.m_maxs.x < secondAABB.m_maxs.x) {
		newMax.x = secondAABB.m_maxs.x;
	}
	else {
		newMax.x = firstAABB.m_maxs.x;
	}
	if (firstAABB.m_maxs.y < secondAABB.m_maxs.y) {
		newMax.y = secondAABB.m_maxs.y;
	}
	else {
		newMax.y = firstAABB.m_maxs.y;
	}
	if (firstAABB.m_maxs.z < secondAABB.m_maxs.z) {
		newMax.z = secondAABB.m_maxs.z;
	}
	else {
		newMax.z = firstAABB.m_maxs.z;
	}

	return AABB3(newMin, newMax);
}

void GetUVOfOneAABB2RelativeToAnother(const AABB2& aabb2ToGetUVOf, const AABB2& criteriaAABB2, Vec2& out_uv_mins, Vec2& out_uv_maxs)
{
	// Calculate the width and height of both AABBs
	float criteriaWidth = criteriaAABB2.m_maxs.x - criteriaAABB2.m_mins.x;
	float criteriaHeight = criteriaAABB2.m_maxs.y - criteriaAABB2.m_mins.y;

	float inv_criteriaWidth = 1.0f / criteriaWidth;
	float inv_criteriaHeight = 1.0f / criteriaHeight;

	// Calculate the UV coordinates relative to the criteriaAABB2
	out_uv_mins.x = (aabb2ToGetUVOf.m_mins.x - criteriaAABB2.m_mins.x) * inv_criteriaWidth;
	out_uv_mins.y = (aabb2ToGetUVOf.m_mins.y - criteriaAABB2.m_mins.y) * inv_criteriaHeight;
	out_uv_maxs.x = (aabb2ToGetUVOf.m_maxs.x - criteriaAABB2.m_mins.x) * inv_criteriaWidth;
	out_uv_maxs.y = (aabb2ToGetUVOf.m_maxs.y - criteriaAABB2.m_mins.y) * inv_criteriaHeight;
}

bool DoesAABB2FitInOtherAABB2(const AABB2& inAABB2, const AABB2& wrapperAABB2)
{
	return ((inAABB2.m_mins.x >= wrapperAABB2.m_mins.x) && (inAABB2.m_maxs.x <= wrapperAABB2.m_maxs.x) && (inAABB2.m_mins.y >= wrapperAABB2.m_mins.y) && (inAABB2.m_maxs.y <= wrapperAABB2.m_maxs.y));
}

AABB2 CreateAABB2FromTwoUnorderedPoints(const Vec2& firstPoint, const Vec2& secondPoint)
{
	// Calculate mins and maxs for the AABB
	Vec2 mins, maxs;
	
	mins.x = GetMin(firstPoint.x, secondPoint.x);
	mins.y = GetMin(firstPoint.y, secondPoint.y);
	
	maxs.x = GetMax(firstPoint.x, secondPoint.x);
	maxs.y = GetMax(firstPoint.y, secondPoint.y);
	
	// Construct and return the AABB2 with calculated mins and maxs
	return AABB2(mins, maxs);
}

Plane3D GetPlaneFromTwoLineSegments(const LineSegment3& firstSegment, const LineSegment3& secondSegment)
{
	GUARANTEE_OR_DIE(firstSegment.GetLength() > 0.0f && secondSegment.GetLength() > 0.0f, "Both segments should have a length longer than 0.0f!");

	Vec3 firstSegmentVec = (firstSegment.m_end - firstSegment.m_start).GetNormalized();
	Vec3 secondSegmentVec = (secondSegment.m_end - secondSegment.m_end).GetNormalized();

	float dotProductForValidation = DotProduct3D(firstSegmentVec, secondSegmentVec);

	GUARANTEE_OR_DIE(dotProductForValidation != 1.0f, "FUCKED UP");

	Vec3 planeNormal = CrossProduct3D(firstSegmentVec, secondSegmentVec);

	return Plane3D(planeNormal, firstSegment.m_start);
}

Plane3D GetPlaneFromTwoVec3sAndPoint(const Vec3& firstVec, const Vec3& secondVec, const Vec3& pointOnPlane)
{
	GUARANTEE_OR_DIE(firstVec.GetLength() > 0.0f && secondVec.GetLength() > 0.0f, "Both Vecs should have a length longer than 0.0f!");

	float dotProductForValidation = DotProduct3D(firstVec, secondVec);

	float debugTemp = firstVec.GetLength() * secondVec.GetLength();
	GUARANTEE_OR_DIE(dotProductForValidation != debugTemp, "FUCKED UP");

	Vec3 planeNormal = CrossProduct3D(firstVec, secondVec).GetNormalized();
	return Plane3D(planeNormal, pointOnPlane);
}

//ONLY works if lineToProjectOnto's direction is a normalized vector!
Vec3 GetProjectedPointOntoInfiniteLine3D(const Vec3& point, const InfiniteLine3& lineToProjectOnto)
{
	Vec3 fromPointOnLineToOuterPoint = point - lineToProjectOnto.m_point;
	Vec3 projectedOntoVec = GetVectorProjectedOnto3D(fromPointOnLineToOuterPoint, lineToProjectOnto.m_direction);
	return lineToProjectOnto.m_point + projectedOntoVec;
}

unsigned int GetCombination(unsigned int n, unsigned int r)
{
	if (r > n)
		return 0;
	if ((r == n) || (r == 0))
		return 1;
	return GetFactorial(n) / (GetFactorial(r) * GetFactorial(n - r));
}

float GetMin(float value1, float value2)
{
	if (value1 <= value2)
		return value1;
	else
		return value2;
}

float GetMax(float value1, float value2)
{
	if (value1 <= value2)
		return value2;
	else
		return value1;
}

int GetMin(int value1, int value2)
{
	if (value1 <= value2)
		return value1;
	else
		return value2;
}

int GetMax(int value1, int value2)
{
	if (value1 <= value2)
		return value2;
	else
		return value1;
}

int GetAbs(int value)
{
	if (value < 0)
		return -value;
	return value;
}

float GetAbsf(float value)
{
	if(value < 0.0f)
		return -value;
	return value;
}

float GetSign(float value)
{
	if (value >= 0.0f)
		return 1.0f;
	else
		return -1.0f;
}

float GetClamped(float value, float minValue, float maxValue)
{
	if (value < minValue)
		return minValue;
	if (value > maxValue)
		return maxValue;
	return value;
}

float GetClampedZeroToOne(float value)
{
	return GetClamped(value, 0.f, 1.f);
}

float Interpolate(float start, float end, float fractionTowardEnd)
{
	return (1.f-fractionTowardEnd) * start + fractionTowardEnd * end;
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	return (value-rangeStart)/(rangeEnd-rangeStart);
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float originalFraction = GetFractionWithinRange(inValue, inStart, inEnd);
	return originalFraction * (outEnd - outStart) + outStart;
}

float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if (inValue < inStart) {
		return outStart;
	}
	if (inValue > inEnd) {
		return outEnd;
	}
	float rangeMapResults = RangeMap(inValue, inStart, inEnd, outStart, outEnd);
	return rangeMapResults;
}

int RangeMap(int inValue, int inStart, int inEnd, int outStart, int outEnd)
{
	// Calculate the input range and output range sizes
	int inRange = inEnd - inStart;
	int outRange = outEnd - outStart;

	// Normalize the input value to the range [0, 1]
	float normalizedValue = static_cast<float>(inValue - inStart) / inRange;

	// Map the normalized value to the output range
	int mappedValue = static_cast<int>(normalizedValue * outRange + outStart);

	return mappedValue;
}

int RangeMapClamped(int inValue, int inStart, int inEnd, int outStart, int outEnd)
{
	if (inValue < inStart) {
		return outStart;
	}
	if (inValue > inEnd) {
		return outEnd;
	}
	int rangeMapResults = RangeMap(inValue, inStart, inEnd, outStart, outEnd);
	return rangeMapResults;
}

int RoundDownToInt(float value)
{
	return static_cast<int>(floorf(value));
}

float Sqrtf(float nonNegativeValue)
{
	return sqrtf(nonNegativeValue);
}

Vec2 Lerp(const Vec2& start, const Vec2& end, float fractionTowardEnd)
{
	return (1.f - fractionTowardEnd) * start + fractionTowardEnd * end;
}

Vec3 Lerp(const Vec3& start, const Vec3& end, float fractionTowardEnd)
{
	return (1.f - fractionTowardEnd) * start + fractionTowardEnd * end;
}

Vec4 Lerp(const Vec4& start, const Vec4& end, float fractionTowardEnd)
{
	return (1.f - fractionTowardEnd) * start + fractionTowardEnd * end;
}

Vec2 Slerp(const Vec2& start, const Vec2& end, float fractionTowardEnd)
{
	float startOri = start.GetOrientationDegrees();
	float endOri = end.GetOrientationDegrees();
	float angularDisp = GetShortestAngularDispDegrees(startOri, endOri);
	float resultOri = startOri + angularDisp * fractionTowardEnd;
	return Vec2::MakeFromPolarDegrees(resultOri);
}
