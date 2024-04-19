#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"

struct AABB3
{
public:
	static const AABB3 ZERO_TO_ONE;

	Vec3 m_mins;
	Vec3 m_maxs;
	//Vec3 m_center;

public:
	~AABB3();
	AABB3();
	AABB3(AABB3 const& copyFrom);
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3(Vec3 const& mins, Vec3 const& maxs);
	explicit AABB3(Vec3 const& center, float xHalfDimension, float yHalfDimension, float zHalfDimension);
	//Get Front Left Bottom
	Vec3 GetFLB() const;
	//Get Front Left Top
	Vec3 GetFLT() const;
	//Get Front Right Top
	Vec3 GetFRT() const; 
	//Get Back Left Bottom
	Vec3 GetBLB() const;
	//Get Back Right Top
	Vec3 GetBRT() const;
	//Get Back Right Bottom
	Vec3 GetBRB() const;

	Vec3 GetCenter() const;

	bool IsPointInside(const Vec3& point) const;
	
	AABB2 GetBoundsXY() const;

	AABB3 GetBoundsXY2D() const;

	float GetBoundingRadius() const;

	Vec3 GetNearestPoint(const Vec3& referencePosition) const;

	void StretchToIncludePoint(Vec3 const& point);

	float GetShortestDimensionSize();
};