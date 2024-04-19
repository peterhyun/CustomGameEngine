#pragma once
#include "Engine/Math/Vec2.hpp"

struct Vec3;
struct Vec4;
struct IntVec2;
struct Mat44;
struct AABB2;
struct AABB3;
struct Capsule2;
struct Capsule3;
struct OBB2;
struct LineSegment2;
struct LineSegment3;
struct FloatRange;
struct ConvexPoly2D;
struct ConvexHull2D;
struct Plane3D;
struct InfiniteLine3;

enum class BillboardType
{
	NONE = -1,
	WORLD_UP_CAMERA_FACING,
	WORLD_UP_CAMERA_OPPOSING,
	FULL_CAMERA_FACING,
	FULL_CAMERA_OPPOSING,
	COUNT
};

float GetMin(float value1, float value2);
float GetMax(float value1, float value2);
int GetMin(int value1, int value2);
int GetMax(int value1, int value2);
int GetAbs(int value);
float GetAbsf(float value);
float GetSign(float value);
float GetClamped(float value, float minValue, float maxValue);
float GetClampedZeroToOne(float value);
float Interpolate(float start, float end, float fractionTowardEnd);
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd);
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
int RangeMap(int inValue, int inStart, int inEnd, int outStart, int outEnd);
int RangeMapClamped(int inValue, int inStart, int inEnd, int outStart, int outEnd);
int RoundDownToInt(float value);
float Sqrtf(float nonNegativeValue);
Vec2 Lerp(const Vec2& start, const Vec2& end, float fractionTowardEnd);
Vec3 Lerp(const Vec3& start, const Vec3& end, float fractionTowardEnd);
Vec4 Lerp(const Vec4& start, const Vec4& end, float fractionTowardEnd);
Vec2 Slerp(const Vec2& start, const Vec2& end, float fractionTowardEnd);

//Angle utilities
float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float CosRadians(float radians);
float SinDegrees(float degrees);
float SinRadians(float radians);
float TanDegrees(float degrees);
float AsinDegrees(float value);
float Atan2Degrees(float y, float x);
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees);
float GetShortestAngularDispRadians(float startRadians, float endRadians);
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);
Vec2 RotatePointAroundAnotherPointDegrees2D(const Vec2& pointToRotate, const Vec2& fixedPoint, float degreesToRotate);
Vec2 ScalePointFromAnotherPoint2D(const Vec2& pointToScale, const Vec2& fixedPoint, float scale);

float DotProduct2D(Vec2 const& a, Vec2 const& b);
float DotProduct3D(Vec3 const& a, Vec3 const& b);
float DotProduct4D(Vec4 const& a, Vec4 const& b);
float CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3  CrossProduct3D(Vec3 const& a, Vec3 const& b);

//Distance & projections utilities
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float GetVectorProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto); //Works even if Vecs not normalized
Vec2 const GetVectorProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto); //Works even if Vecs not normalized
float GetVectorProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto); //Works even if Vecs not normalized
Vec3 const GetVectorProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto); //Works even if Vecs not normalized
float GetDistanceBetweenLineSegmentAndPoint2D(const LineSegment2& lineSegment, const Vec2& point);
float GetDistanceSquaredBetweenLineSegmentAndPoint2D(const LineSegment2& lineSegment, const Vec2& point);
float GetDistanceBetweenLineSegmentAndPoint3D(const LineSegment3& lineSegment, const Vec3& point);
float GetDistanceSquaredBetweenLineSegmentAndPoint3D(const LineSegment3& lineSegment, const Vec3& point);

//Geometric queries
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule);
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool IsPointInsideCapsule3D(Vec3 const& point, Capsule3 const& capsule);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox);
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideSphere(const Vec3& point, const Vec3& sphereCenter, float sphereRadius);
bool IsPointInsideConvexPoly2D(const Vec2& point, const ConvexPoly2D& convexPoly);
bool IsPointInsideConvexHull2D(const Vec2& point, const ConvexHull2D& convexHull);

bool DoAABB2sOverlap(const AABB2& first, const AABB2& second);
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoDiscAndAABB2DOverlap(Vec2 const& discCenter, float discRadius, const AABB2& aabb2);
bool DoDiscAndOBB2Overlap(Vec2 const& discCenter, float discRadius, const OBB2& obb2);
bool DoDiscAndCapsule2Overlap(Vec2 const& discCenter, float discRadius, const Capsule2& capsule2);
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoZCylindersOverlap(Vec3 const& bottomCenterA, float heightA, float radiusA, Vec3 const& bottomCenterB, float heightB, float radiusB);
bool DoZCylinderAndAABB3DOverlap(Vec3 const& cylinderBottomCenter, float cylinderRadius, const FloatRange& cylinderZRange, const AABB3& aabb3);
bool DoAABB3AndSphereOverlap(const AABB3& aabb, const Vec3& sphereCenter, float sphereRadius);

Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius);
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2& box);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, LineSegment2 const& lineSegment);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 const& capsule);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox);
Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePos, LineSegment3 const& lineSegment);
Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePos, Vec3 const& lineSegStart, Vec3 const& lineSegEnd);

bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint);
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);
bool PushDiscOutOfFixedOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& fixedOBB2);
bool PushDiscOutOfFixedCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& fixedCapsule);

bool BounceDiscsOffEachOther2D(Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA = 1.0f, float elasticityB = 1.0f);
bool BounceDiscOffOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocityA, const Vec2& fixedDiscCenter, float fixedDiscRadius, float mobileDiscElasticity = 1.0f, float fixedDiscElasticity = 1.0f);
bool BounceDiscOffOfFixedOBB2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, const OBB2& fixedOBB2, float mobileDiscElasticity, float fixedOBB2Elasticity);
bool BounceDiscOffOfFixedCapsule2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, const Capsule2& fixedCapsule, float mobileDiscElasticity, float fixedCapsuleElasticity);

//Transformation utilities
void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation);
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);

//GetLookAtMatrix
Mat44 GetBillboardMatrix(BillboardType billboardType, const Mat44& targetMatrix, const Vec3& billboardPosition, const Vec2& billboardScale = Vec2(1.0f, 1.0f));

float		  NormalizeByte(unsigned char byteValue);
unsigned char DenormalizeByte(float zeroToOne);

//Curve functions
float ComputeCubicBezier1D(float A, float B, float C, float D, float t);
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t);
float ComputeCubicBezierTangent1D(float A, float B, float C, float D, float t);

float SmoothStart2(float input);
float SmoothStart3(float input);
float SmoothStart4(float input);
float SmoothStart5(float input);
float SmoothStart6(float input);
float SmoothStop2(float input);
float SmoothStop3(float input);
float SmoothStop4(float input);
float SmoothStop5(float input);
float SmoothStop6(float input);
float SmoothStep3(float input);
float SmoothStep5(float input);
float Hesitate3(float input);
float Hesitate5(float input);
float CustomFunkyEasingFunction(float input);

unsigned int GetCombination(unsigned int n, unsigned int r);
unsigned int GetFactorial(unsigned int n);

//BVH tree node
AABB2 MergeAABB2s(const AABB2& firstAABB, const AABB2& secondAABB);

//BVH tree node
AABB3 MergeAABB3s(const AABB3& firstAABB, const AABB3& secondAABB);

//For DFS2 Editor
void GetUVOfOneAABB2RelativeToAnother(const AABB2& aabb2ToGetUVOf, const AABB2& criteriaAABB2, Vec2& out_uv_mins, Vec2& out_uv_maxs);

bool DoesAABB2FitInOtherAABB2(const AABB2& inAABB2, const AABB2& wrapperAABB2);

AABB2 CreateAABB2FromTwoUnorderedPoints(const Vec2& firstPoint, const Vec2& secondPoint);

Plane3D GetPlaneFromTwoLineSegments(const LineSegment3& firstSegment, const LineSegment3& secondSegment);
Plane3D GetPlaneFromTwoVec3sAndPoint(const Vec3& firstVec, const Vec3& secondVec, const Vec3& pointOnPlane);

Vec3 GetProjectedPointOntoInfiniteLine3D(const Vec3& point, const InfiniteLine3& lineToProjectOnto);