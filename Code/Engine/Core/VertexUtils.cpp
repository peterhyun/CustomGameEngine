#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/CubicBezierCurve2.hpp"
#include "Engine/Math/CubicHermiteCurve2.hpp"
#include "Engine/Math/CatmullRomSpline2.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include <map>

void TransformVertexArrayXY3D(const int num_vertices, Vertex_PCU* const tempWorldVerts, float scaleXY, const float zRotationDegrees, const Vec2& translationXY)
{
	for (int vertexIndex = 0; vertexIndex < num_vertices; vertexIndex++) {
		TransformPositionXY3D(tempWorldVerts[vertexIndex].m_position, scaleXY, zRotationDegrees, translationXY);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform)
{
	for (int vertexIndex = 0; vertexIndex < verts.size() ; vertexIndex++) {
		verts[vertexIndex].m_position = transform.TransformPosition3D(verts[vertexIndex].m_position);
	}
}

AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts)
{
	if (verts.size() == 0) {
		ERROR_AND_DIE("vertex array has to be populated in GetVertexBounds2D()");
	}
	float minX = verts[0].m_position.x;
	float maxX = minX;
	float minY = verts[0].m_position.y;
	float maxY = minY;
	for (int i = 1; i < verts.size() ; i++) {
		if (verts[i].m_position.x < minX) {
			minX = verts[i].m_position.x;
		}
		if (verts[i].m_position.y < minY) {
			minY = verts[i].m_position.y;
		}
		if (verts[i].m_position.x > maxX) {
			maxX = verts[i].m_position.x;
		}
		if (verts[i].m_position.y > maxY) {
			maxY = verts[i].m_position.y;
		}
	}
	return AABB2(minX, minY, maxX, maxY);
}

void AddVertsForAABB2(std::vector<Vertex_PCU>& verts, const AABB2& bounds, const Rgba8& color, const AABB2& uvs)
{
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs), color, uvs.m_maxs));
	verts.push_back(Vertex_PCU(Vec3(bounds.GetTopLeft(), 0.0f), color, uvs.GetTopLeft()));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins), color, uvs.m_mins));

	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs), color, uvs.m_maxs));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins), color, uvs.m_mins));
	verts.push_back(Vertex_PCU(Vec3(bounds.GetBottomRight()), color, uvs.GetBottomRight()));
}

void AddVertsForWireframeAABB2(std::vector<Vertex_PCU>& verts, const AABB2& bounds, float thickness, const Rgba8& color)
{
	AddVertsForLineSegment2D(verts, bounds.m_mins, bounds.GetBottomRight(), thickness, color);
	AddVertsForLineSegment2D(verts, bounds.GetBottomRight(), bounds.m_maxs, thickness, color);
	AddVertsForLineSegment2D(verts, bounds.m_maxs, bounds.GetTopLeft(), thickness, color);
	AddVertsForLineSegment2D(verts, bounds.GetTopLeft(), bounds.m_mins, thickness, color);
}

void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, const AABB2& uvs)
{
	Vec2 cornerPoints[4];
	box.GetCornerPoints(cornerPoints);
	verts.push_back(Vertex_PCU(Vec3(cornerPoints[0]), color, uvs.GetTopLeft()));
	verts.push_back(Vertex_PCU(Vec3(cornerPoints[1]), color, uvs.m_mins));
	verts.push_back(Vertex_PCU(Vec3(cornerPoints[3]), color, uvs.m_maxs));

	verts.push_back(Vertex_PCU(Vec3(cornerPoints[3]), color, uvs.m_maxs));
	verts.push_back(Vertex_PCU(Vec3(cornerPoints[1]), color, uvs.m_mins));
	verts.push_back(Vertex_PCU(Vec3(cornerPoints[2]), color, uvs.GetBottomRight()));
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	Vec2 fromStartToEnd = boneEnd - boneStart;
	fromStartToEnd.Normalize();
	Vec2 forwardDir = fromStartToEnd * radius;
	Vec2 leftStep = forwardDir.GetRotated90Degrees();

	Vec3 S_L(boneStart + leftStep, 0.f);
	Vec3 S_R(boneStart - leftStep, 0.f);
	Vec3 E_L(boneEnd + leftStep, 0.f);
	Vec3 E_R(boneEnd - leftStep, 0.f);

	verts.push_back(Vertex_PCU(S_L, color));
	verts.push_back(Vertex_PCU(S_R, color));
	verts.push_back(Vertex_PCU(E_R, color));

	verts.push_back(Vertex_PCU(E_R, color));
	verts.push_back(Vertex_PCU(E_L, color));
	verts.push_back(Vertex_PCU(S_L, color));

	//Draw a half disc
	constexpr int NUM_SIDES = 24;
	constexpr float DEGREES_PER_SIDE = 180.f / static_cast<float> (NUM_SIDES);
	const float leftDiscStartDegrees = leftStep.GetOrientationDegrees();
	const float rightDiscStartDegrees = leftDiscStartDegrees + 180.0f;

	for (int sideIndex = 0; sideIndex < NUM_SIDES; sideIndex++) {
		Vec3 leftHalfDiscRight(boneStart.x + (radius * CosDegrees(leftDiscStartDegrees + DEGREES_PER_SIDE * sideIndex)), boneStart.y + (radius * SinDegrees(leftDiscStartDegrees + DEGREES_PER_SIDE * sideIndex)), 0.f);
		Vec3 leftHalfDiscLeft(boneStart.x + (radius * CosDegrees(leftDiscStartDegrees + DEGREES_PER_SIDE * (sideIndex + 1))), boneStart.y + (radius * SinDegrees(leftDiscStartDegrees + DEGREES_PER_SIDE * (sideIndex + 1))), 0.f);
		verts.push_back(Vertex_PCU(leftHalfDiscRight, color));
		verts.push_back(Vertex_PCU(leftHalfDiscLeft, color));
		verts.push_back(Vertex_PCU(boneStart, color));

		Vec3 rightHalfDiscRight(boneEnd.x + (radius * CosDegrees(rightDiscStartDegrees + DEGREES_PER_SIDE * sideIndex)), boneEnd.y + (radius * SinDegrees(rightDiscStartDegrees + DEGREES_PER_SIDE * sideIndex)), 0.f);
		Vec3 rightHalfDiscLeft(boneEnd.x + (radius * CosDegrees(rightDiscStartDegrees + DEGREES_PER_SIDE * (sideIndex + 1))), boneEnd.y + (radius * SinDegrees(rightDiscStartDegrees + DEGREES_PER_SIDE * (sideIndex + 1))), 0.f);
		verts.push_back(Vertex_PCU(rightHalfDiscRight, color));
		verts.push_back(Vertex_PCU(rightHalfDiscLeft, color));
		verts.push_back(Vertex_PCU(boneEnd, color));
	}
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color)
{
	AddVertsForCapsule2D(verts, capsule.GetStartPos(), capsule.GetEndPos(), capsule.GetRadius(), color);
}

void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color)
{
	constexpr int NUM_SIDES = 48;
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float> (NUM_SIDES);
	for (int sideIndex = 0; sideIndex < NUM_SIDES; sideIndex++) {
		Vec3 right(center.x + (radius * CosDegrees(DEGREES_PER_SIDE * sideIndex)), center.y + (radius * SinDegrees(DEGREES_PER_SIDE * sideIndex)), 0.f);
		Vec3 left(center.x + (radius * CosDegrees(DEGREES_PER_SIDE * (sideIndex + 1))), center.y + (radius * SinDegrees(DEGREES_PER_SIDE * (sideIndex + 1))), 0.f);
		verts.push_back(Vertex_PCU(left, color));
		verts.push_back(Vertex_PCU(center, color));
		verts.push_back(Vertex_PCU(right, color));
	}
}

void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec2& center, float innerRadius, float outerRadius, const Rgba8& color, const AABB2& UVs)
{
	if (innerRadius >= outerRadius || innerRadius == 0.0f || outerRadius == 0.0f) {
		ERROR_AND_DIE("WRONG input for AddVertsForRingZ3D()");
	}
	int numTriangles = 2 * numSlices;
	int numVertices = 3 * numTriangles;
	float degreesPerSide = 360.f / static_cast<float> (numSlices);
	std::vector<Vertex_PCU> vertices;
	vertices.resize(numVertices);

	float innerOuterRadiusRatio = innerRadius / outerRadius;
	AABB2 smallUVBox = UVs.GetBoxWithin(Vec2(0.5f - innerOuterRadiusRatio * 0.5f, 0.5f - innerOuterRadiusRatio * 0.5f), Vec2(0.5f + innerOuterRadiusRatio * 0.5f, 0.5f + innerOuterRadiusRatio * 0.5f));

	for (int sideIndex = 0; sideIndex < numSlices; sideIndex++) {
		float cosSliceIndex = CosDegrees(degreesPerSide * sideIndex);
		float cosSliceIndex_1 = CosDegrees(degreesPerSide * (sideIndex + 1));
		float sinSliceIndex = SinDegrees(degreesPerSide * sideIndex);
		float sinSliceIndex_1 = SinDegrees(degreesPerSide * (sideIndex + 1));

		Vec3 I_L(center.x + (innerRadius * cosSliceIndex_1), center.y + (innerRadius * sinSliceIndex_1), 0.f);
		Vec3 I_R(center.x + (innerRadius * cosSliceIndex), center.y + (innerRadius * sinSliceIndex), 0.f);
		Vec2 rightInnerUV = Vec2(RangeMap(cosSliceIndex, -1.0f, 1.0f, smallUVBox.m_mins.x, smallUVBox.m_maxs.x), RangeMap(sinSliceIndex, -1.0f, 1.0f, smallUVBox.m_mins.y, smallUVBox.m_maxs.y));
		Vec2 leftInnerUV = Vec2(RangeMap(cosSliceIndex_1, -1.0f, 1.0f, smallUVBox.m_mins.x, smallUVBox.m_maxs.x), RangeMap(sinSliceIndex_1, -1.0f, 1.0f, smallUVBox.m_mins.y, smallUVBox.m_maxs.y));

		Vec3 O_R(center.x + (outerRadius * cosSliceIndex), center.y + (outerRadius * sinSliceIndex), 0.f);
		Vec3 O_L(center.x + (outerRadius * cosSliceIndex_1), center.y + (outerRadius * sinSliceIndex_1), 0.f);
		Vec2 rightOuterUV = Vec2(RangeMap(cosSliceIndex, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vec2 leftOuterUV = Vec2(RangeMap(cosSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));

		vertices[6 * sideIndex + 0].m_position = I_L;
		vertices[6 * sideIndex + 1].m_position = I_R;
		vertices[6 * sideIndex + 2].m_position = O_R;

		vertices[6 * sideIndex + 3].m_position = I_L;
		vertices[6 * sideIndex + 4].m_position = O_R;
		vertices[6 * sideIndex + 5].m_position = O_L;

		vertices[6 * sideIndex + 0].m_color = color;
		vertices[6 * sideIndex + 1].m_color = color;
		vertices[6 * sideIndex + 2].m_color = color;

		vertices[6 * sideIndex + 3].m_color = color;
		vertices[6 * sideIndex + 4].m_color = color;
		vertices[6 * sideIndex + 5].m_color = color;

		vertices[6 * sideIndex + 0].m_uvTexCoords = leftInnerUV;
		vertices[6 * sideIndex + 1].m_uvTexCoords = rightInnerUV;
		vertices[6 * sideIndex + 2].m_uvTexCoords = rightOuterUV;

		vertices[6 * sideIndex + 3].m_uvTexCoords = leftInnerUV;
		vertices[6 * sideIndex + 4].m_uvTexCoords = rightOuterUV;
		vertices[6 * sideIndex + 5].m_uvTexCoords = leftOuterUV;
	}
	verts.insert(verts.end(), vertices.begin(), vertices.end());
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 fromStartToEnd = end - start;
	fromStartToEnd.Normalize();
	float radius = thickness * 0.5f;
	Vec2 forwardDir = fromStartToEnd * radius;
	Vec2 leftStep = forwardDir.GetRotated90Degrees();

	Vec3 S_L(start + leftStep, 0.f);
	Vec3 S_R(start - leftStep, 0.f);
	Vec3 E_L(end + leftStep, 0.f);
	Vec3 E_R(end - leftStep, 0.f);

	verts.push_back(Vertex_PCU(S_L, color));
	verts.push_back(Vertex_PCU(S_R, color));
	verts.push_back(Vertex_PCU(E_R, color));

	verts.push_back(Vertex_PCU(E_R, color));
	verts.push_back(Vertex_PCU(E_L, color));
	verts.push_back(Vertex_PCU(S_L, color));
}

void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color)
{
	AddVertsForLineSegment2D(verts, lineSegment.m_start, lineSegment.m_end, thickness, color);
}

void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float thickness, const Rgba8& color)
{
	Vec3 fromStartToEnd = (end - start).GetNormalized();
	Vec3 worldUp(0.0f, 0.0f, 1.0f);
	Vec3 perpendicularToFromStartToEnd1;
	Vec3 perpendicularToFromStartToEnd2;
	if (!(fromStartToEnd.x == 0.0f && fromStartToEnd.y == 0.0f)) {	// Comparing fromStartToEnd directly to worldUp sometimes would cause some bugs due to float imprecision
		perpendicularToFromStartToEnd1 = CrossProduct3D(fromStartToEnd, worldUp);
		perpendicularToFromStartToEnd1.Normalize();
		perpendicularToFromStartToEnd2 = CrossProduct3D(perpendicularToFromStartToEnd1, fromStartToEnd);
		perpendicularToFromStartToEnd2.Normalize();
	}
	else {
		perpendicularToFromStartToEnd1 = Vec3(0.0f, -1.0f, 0.0f);
		perpendicularToFromStartToEnd2 = Vec3(-1.0f, 0.0f, 0.0f);
	}
	float halfThickness = thickness * 0.5f;

	//Start/End_Top/Bottom_Right/Left
	Vec3 STR(start + perpendicularToFromStartToEnd1 * halfThickness + perpendicularToFromStartToEnd2 * halfThickness);
	Vec3 STL(start - perpendicularToFromStartToEnd1 * halfThickness + perpendicularToFromStartToEnd2 * halfThickness);
	Vec3 SBR(start + perpendicularToFromStartToEnd1 * halfThickness - perpendicularToFromStartToEnd2 * halfThickness);
	Vec3 SBL(start - perpendicularToFromStartToEnd1 * halfThickness - perpendicularToFromStartToEnd2 * halfThickness);

	Vec3 ETR(end + perpendicularToFromStartToEnd1 * halfThickness + perpendicularToFromStartToEnd2 * halfThickness);
	Vec3 ETL(end - perpendicularToFromStartToEnd1 * halfThickness + perpendicularToFromStartToEnd2 * halfThickness);
	Vec3 EBR(end + perpendicularToFromStartToEnd1 * halfThickness - perpendicularToFromStartToEnd2 * halfThickness);
	Vec3 EBL(end - perpendicularToFromStartToEnd1 * halfThickness - perpendicularToFromStartToEnd2 * halfThickness);

	//forward face
	AddVertsForQuad3D(verts, SBL, SBR, STR, STL, color);
	//backward face
	AddVertsForQuad3D(verts, ETL, ETR, EBR, EBL, color);
	//right face
	AddVertsForQuad3D(verts, SBR, EBR, ETR, STR, color);
	//left face
	AddVertsForQuad3D(verts, STL, ETL, EBL, SBL, color);
	//top face
	AddVertsForQuad3D(verts, STL, STR, ETR, ETL, color);
	//bottom face
	AddVertsForQuad3D(verts, EBL, EBR, SBR, SBL, color);
}

void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	AddVertsForLineSegment2D(verts, tipPos, tailPos, lineThickness, color);
	Vec2 fromTipToTailDir = (tailPos - tipPos).GetNormalized();
	AddVertsForLineSegment2D(verts, tipPos, tipPos + (fromTipToTailDir * arrowSize) + (fromTipToTailDir * arrowSize).GetRotated90Degrees() * 0.5f, lineThickness, color);
	AddVertsForLineSegment2D(verts, tipPos, tipPos + (fromTipToTailDir * arrowSize) - (fromTipToTailDir * arrowSize).GetRotated90Degrees() * 0.5f, lineThickness, color);
}

void AddVertsForCubicBezierCurve2D(std::vector<Vertex_PCU>& verts, const CubicBezierCurve2& cubicBezierCurve, int subdivisions, float lineThickness, const Rgba8& color)
{
	float inv_subdivisions = 1.0f / (float)subdivisions;
	float startTForLineSegment = 0.0f;
	for (int i = 0; i < subdivisions; i++) {
		float endTForLineSegment = inv_subdivisions * (float)(i + 1);
		AddVertsForLineSegment2D(verts,
			cubicBezierCurve.EvaluateAtParametric(startTForLineSegment),
			cubicBezierCurve.EvaluateAtParametric(endTForLineSegment),
			lineThickness,
			color);
		startTForLineSegment = endTForLineSegment;
	}
}

void AddVertsForCubicHermiteCurve2D(std::vector<Vertex_PCU>& verts, const CubicHermiteCurve2& cubicHermiteCurve, int subdivisions, float lineThickness, const Rgba8& color)
{
	float inv_subdivisions = 1.0f / (float)subdivisions;
	float startTForLineSegment = 0.0f;
	for (int i = 0; i < subdivisions; i++) {
		float endTForLineSegment = inv_subdivisions * (float)(i + 1);
		AddVertsForLineSegment2D(verts,
			cubicHermiteCurve.EvaluateAtParametric(startTForLineSegment),
			cubicHermiteCurve.EvaluateAtParametric(endTForLineSegment),
			lineThickness,
			color);
		startTForLineSegment = endTForLineSegment;
	}
}

void AddVertsForCatmullRomSpline2D(std::vector<Vertex_PCU>& verts, const CatmullRomSpline2& catmullRomSpline, int subdivisions, float lineThickness, const Rgba8& color)
{
	for (int i = 0; i < catmullRomSpline.m_cubicHermiteCurves.size(); i++) {
		AddVertsForCubicHermiteCurve2D(verts, catmullRomSpline.m_cubicHermiteCurves[i], subdivisions, lineThickness, color);
	}
}

void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, const ConvexPoly2D& convexPoly, const Rgba8& color)
{
	//Add like a fan
	const std::vector<Vec2>& points = convexPoly.m_points;
	unsigned int size = (unsigned int)points.size();
	GUARANTEE_OR_DIE(size >= 3, "convexPoly should be at least a triangle to add verts for it");
	for (unsigned int i = 0; i < size -  2; i++) {
		verts.push_back(Vertex_PCU(points[0], color));
		verts.push_back(Vertex_PCU(points[i + 1], color));
		verts.push_back(Vertex_PCU(points[i + 2], color));
	}
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3 topLeft, const Rgba8& color, const AABB2& UVs)
{
	verts.push_back(Vertex_PCU(topLeft, color, UVs.GetTopLeft()));
	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
	verts.push_back(Vertex_PCU(topRight, color, UVs.m_maxs));

	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
	verts.push_back(Vertex_PCU(bottomRight, color, UVs.GetBottomRight()));
	verts.push_back(Vertex_PCU(topRight, color, UVs.m_maxs));
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color, const AABB2& UVs)
{
	unsigned int indexOffset = (unsigned int)verts.size();

	verts.push_back(Vertex_PCU(topLeft, color, UVs.GetTopLeft()));
	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
	verts.push_back(Vertex_PCU(bottomRight, color, UVs.GetBottomRight()));
	verts.push_back(Vertex_PCU(topRight, color, UVs.m_maxs));

	indices.push_back(indexOffset + 0);
	indices.push_back(indexOffset + 1);
	indices.push_back(indexOffset + 3);

	indices.push_back(indexOffset + 1);
	indices.push_back(indexOffset + 2);
	indices.push_back(indexOffset + 3);
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3 topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 normalDirection = CrossProduct3D(bottomLeft - topLeft, topRight - topLeft).GetNormalized();
	verts.push_back(Vertex_PCUTBN(topLeft, normalDirection, color, UVs.GetTopLeft()));
	verts.push_back(Vertex_PCUTBN(bottomLeft, normalDirection, color, UVs.m_mins));
	verts.push_back(Vertex_PCUTBN(topRight, normalDirection, color, UVs.m_maxs));

	verts.push_back(Vertex_PCUTBN(bottomLeft, normalDirection, color, UVs.m_mins));
	verts.push_back(Vertex_PCUTBN(bottomRight, normalDirection, color, UVs.GetBottomRight()));
	verts.push_back(Vertex_PCUTBN(topRight, normalDirection, color, UVs.m_maxs));
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color, const AABB2& UVs)
{
	unsigned int indexOffset = (unsigned int)verts.size();

	Vec3 normalDirection = CrossProduct3D(bottomLeft - topLeft, topRight - topLeft).GetNormalized();
	verts.push_back(Vertex_PCUTBN(topLeft, normalDirection, color, UVs.GetTopLeft()));
	verts.push_back(Vertex_PCUTBN(bottomLeft, normalDirection, color, UVs.m_mins));
	verts.push_back(Vertex_PCUTBN(bottomRight, normalDirection, color, UVs.GetBottomRight()));
	verts.push_back(Vertex_PCUTBN(topRight, normalDirection, color, UVs.m_maxs));

	indices.push_back(indexOffset + 0);
	indices.push_back(indexOffset + 1);
	indices.push_back(indexOffset + 3);

	indices.push_back(indexOffset + 1);
	indices.push_back(indexOffset + 2);
	indices.push_back(indexOffset + 3);
}

void AddVertsForWireframeQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3 topLeft, float thickness, const Rgba8& color)
{
	AddVertsForLineSegment3D(verts, bottomLeft, bottomRight, thickness, color);
	AddVertsForLineSegment3D(verts, bottomRight, topRight, thickness, color);
	AddVertsForLineSegment3D(verts, topRight, topLeft, thickness, color);
	AddVertsForLineSegment3D(verts, topLeft, bottomLeft, thickness, color);
}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	//Back/Front_Right/Left_Bottom/Top (Think of where your eye is)
	Vec3 vFRB(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 vFRT(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 vFLB(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 vFLT(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 vBRB(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 vBRT(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 vBLB(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 vBLT(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

	//Add forward face
	AddVertsForQuad3D(verts, vFLB, vFRB, vFRT, vFLT, color, UVs);
	//Add backward face
	AddVertsForQuad3D(verts, vBLT, vBRT, vBRB, vBLB, color, AABB2(UVs.m_maxs, UVs.m_mins));
	//Add right face
	AddVertsForQuad3D(verts, vFRB, vBRB, vBRT, vFRT, color, UVs);
	//Add left face
	AddVertsForQuad3D(verts, vFLT, vBLT, vBLB, vFLB, color, AABB2(UVs.m_maxs, UVs.m_mins));

	//Add top face
	//AddVertsForQuad3D(verts, vFLT, vFRT, vBRT, vBLT, color, AABB2(UVs.GetTopLeft(), UVs.GetBottomRight())); //<- This doesn't work because the bottom right vertex is always mapped to the bottom right of the UVs. Therefore the texture just gets flipped, instead of getting rotated.
	AddVertsForQuad3D(verts, vFRT, vBRT, vBLT, vFLT, color, UVs);
	
	//Add bottom face
	//AddVertsForQuad3D(verts, vBLB, vBRB, vFRB, vFLB, color, UVs);
	AddVertsForQuad3D(verts, vFLB, vBLB, vBRB, vFRB, color, UVs);
}

void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	//Back/Front_Right/Left_Bottom/Top (Think of where your eye is)
	Vec3 vFRB(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 vFRT(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 vFLB(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 vFLT(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 vBRB(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 vBRT(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 vBLB(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 vBLT(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

	//Add forward face
	AddVertsForQuad3D(verts, indices, vFLB, vFRB, vFRT, vFLT, color, UVs);
	//Add backward face
	AddVertsForQuad3D(verts, indices, vBLT, vBRT, vBRB, vBLB, color, AABB2(UVs.m_maxs, UVs.m_mins));
	//Add right face
	AddVertsForQuad3D(verts, indices, vFRB, vBRB, vBRT, vFRT, color, UVs);
	//Add left face
	AddVertsForQuad3D(verts, indices, vFLT, vBLT, vBLB, vFLB, color, AABB2(UVs.m_maxs, UVs.m_mins));

	//Add top face
	//AddVertsForQuad3D(verts, vFLT, vFRT, vBRT, vBLT, color, AABB2(UVs.GetTopLeft(), UVs.GetBottomRight())); //<- This doesn't work because the bottom right vertex is always mapped to the bottom right of the UVs. Therefore the texture just gets flipped, instead of getting rotated.
	AddVertsForQuad3D(verts, indices, vFRT, vBRT, vBLT, vFLT, color, UVs);

	//Add bottom face
	//AddVertsForQuad3D(verts, vBLB, vBRB, vFRB, vFLB, color, UVs);
	AddVertsForQuad3D(verts, indices, vFLB, vBLB, vBRB, vFRB, color, UVs);
}

void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, const Vec2& centerXY, FloatRange const& minmaxZ, float radius, int numSlices, Rgba8 const& tint, const AABB2& UVs)
{
	float degreesPerSlice = 360.f / numSlices;
	Vec3 centerBottom(centerXY, minmaxZ.m_min);
	Vec3 centerTop(centerXY, minmaxZ.m_max);
	for (int sliceIndex = 0; sliceIndex < numSlices; sliceIndex++) {
		float cosSliceIndex = CosDegrees(degreesPerSlice * sliceIndex);
		float sinSliceIndex = SinDegrees(degreesPerSlice * sliceIndex);
		float cosSliceIndex_1 = CosDegrees(degreesPerSlice * (sliceIndex + 1));
		float sinSliceIndex_1 = SinDegrees(degreesPerSlice * (sliceIndex + 1));

		Vec3 rightBottom(centerXY.x + (radius * cosSliceIndex), centerXY.y + (radius * sinSliceIndex), minmaxZ.m_min);
		Vec3 leftBottom(centerXY.x + (radius * cosSliceIndex_1), centerXY.y + (radius * sinSliceIndex_1), minmaxZ.m_min);
		Vec3 rightTop(centerXY.x + (radius * cosSliceIndex), centerXY.y + (radius * sinSliceIndex), minmaxZ.m_max);
		Vec3 leftTop(centerXY.x + (radius * cosSliceIndex_1), centerXY.y + (radius * sinSliceIndex_1), minmaxZ.m_max);

		Vec2 rightTopUV = Vec2(RangeMap(cosSliceIndex, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vec2 leftTopUV = Vec2(RangeMap(cosSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vec2 rightBottomUV(rightTopUV.x, 1.0f - rightTopUV.y);
		Vec2 leftBottomUV(leftTopUV.x, 1.0f - leftTopUV.y);

		verts.push_back(Vertex_PCU(leftTop, tint, leftTopUV));
		verts.push_back(Vertex_PCU(centerTop, tint, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(rightTop, tint, rightTopUV));
		verts.push_back(Vertex_PCU(rightBottom, tint, rightBottomUV));
		verts.push_back(Vertex_PCU(centerBottom, tint, Vec2(0.5f, 0.5f)));
		verts.push_back(Vertex_PCU(leftBottom, tint, leftBottomUV));

		AABB2 quadUVs;
		quadUVs.m_mins = Vec2(RangeMap(degreesPerSlice * sliceIndex, 0.0f, 360.0f, 0.0f, 1.0f), 0.0f);
		quadUVs.m_maxs = Vec2(RangeMap(degreesPerSlice * (sliceIndex + 1), 0.0f, 360.0f, 0.0f, 1.0f), 1.0f);
		AddVertsForQuad3D(verts, rightBottom, leftBottom, leftTop, rightTop, tint, quadUVs);
	}
}

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numLatitudeSlices)
{
	int numLongtitudeSlices = 2 * numLatitudeSlices;

	//Lets first draw the top half of the sphere
	float latitudeDegrees = 180.0f / numLatitudeSlices;
	float longtitudeDegrees = 360.0f / numLongtitudeSlices;
	for (int i = 0; i < numLatitudeSlices; i++) {
		for (int j = 0; j < numLongtitudeSlices ; j++) {
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * j, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * j, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * (j + 1), radius);
			Vec2 uvBL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvBR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			verts.push_back(Vertex_PCU(BL, color, uvBL));
			verts.push_back(Vertex_PCU(TR, color, uvTR));
			verts.push_back(Vertex_PCU(TL, color, uvTL));

			verts.push_back(Vertex_PCU(TR, color, uvTR));
			verts.push_back(Vertex_PCU(BL, color, uvBL));
			verts.push_back(Vertex_PCU(BR, color, uvBR));
		}
	}
}

void AddVertsForUVSphereZ3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& tint, const AABB2& UVs)
{
	//Lets first draw the top half of the sphere
	float latitudeDegrees = 180.0f / numStacks;
	float longtitudeDegrees = 360.0f / numSlices;
	for (int i = 0; i < numStacks; i++) {
		for (int j = 0; j < numSlices; j++) {
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * j, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * j, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * (j + 1), radius);
			Vec2 uvBL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvBR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			verts.push_back(Vertex_PCU(BL, tint, uvBL));
			verts.push_back(Vertex_PCU(TR, tint, uvTR));
			verts.push_back(Vertex_PCU(TL, tint, uvTL));

			verts.push_back(Vertex_PCU(TR, tint, uvTR));
			verts.push_back(Vertex_PCU(BL, tint, uvBL));
			verts.push_back(Vertex_PCU(BR, tint, uvBR));
		}
	}
}

void AddVertsForSphere3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& color, const AABB2& UVs)
{
	unsigned int indexOffset = (unsigned int)verts.size();
	std::vector<Vertex_PCUTBN> tempVerts;

	//Lets first draw the top half of the sphere
	float latitudeDegrees = 180.0f / numStacks;
	float longtitudeDegrees = 360.0f / numSlices;

	std::map<Vertex_PCUTBN, unsigned int> verticesToIndicesMap;
	auto FigureOutWhereThisVertBelongs = [&](const Vertex_PCUTBN& vertex) {
		auto foundPair = verticesToIndicesMap.find(vertex);
		if (foundPair != verticesToIndicesMap.end()) {
			indices.push_back(foundPair->second);
		}
		else {
			unsigned int newIndex = indexOffset + static_cast<unsigned int>(tempVerts.size());
			verticesToIndicesMap[vertex] = newIndex;
			tempVerts.push_back(vertex);
			indices.push_back(newIndex);
		}
	};

	//1. edge case: i = 0 (Add bottom)
	Vertex_PCUTBN bottom(center + Vec3(0.0f, 0.0f, -radius), Vec3(0.0f, 0.0f, -1.0f), color, UVs.m_mins);	
	for (int j = 0; j < numSlices; j++) {
		Vec3 TL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees, longtitudeDegrees * j, radius);
		Vec3 TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees, longtitudeDegrees * (j + 1), radius);
		Vec2 uvTR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		if (j == numSlices - 1) {
			//To avoid float imprecision
			TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees, 0.0f, radius);
			uvTR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		}
		Vec2 uvTL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vertex_PCUTBN trVertex(TR, (TR - center).GetNormalized(), color, uvTR);
		Vertex_PCUTBN tlVertex(TL, (TL - center).GetNormalized(), color, uvTL);
		FigureOutWhereThisVertBelongs(bottom);
		FigureOutWhereThisVertBelongs(trVertex);
		FigureOutWhereThisVertBelongs(tlVertex);
	}

	//2. edge case: i = numStacks - 1 (Add top)
	Vertex_PCUTBN top(center + Vec3(0.0f, 0.0f, radius), Vec3(0.0f, 0.0f, 1.0f), color, UVs.m_maxs);
	for (int j = 0; j < numSlices; j++) {
		Vec3 BL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (numStacks - 1), longtitudeDegrees * j, radius);
		Vec3 BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (numStacks - 1), longtitudeDegrees * (j + 1), radius);
		Vec2 uvBR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (numStacks - 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		if (j == numSlices - 1) {
			BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (numStacks - 1), 0.0f, radius);
			uvBR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (numStacks - 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		}
		Vec2 uvBL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (numStacks - 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vertex_PCUTBN blVertex(BL, (BL - center).GetNormalized(), color, uvBL);
		Vertex_PCUTBN brVertex(BR, (BR - center).GetNormalized(), color, uvBR);
		FigureOutWhereThisVertBelongs(top);
		FigureOutWhereThisVertBelongs(blVertex);
		FigureOutWhereThisVertBelongs(brVertex);
	}

	for (int i = 1; i < numStacks - 1; i++) {
		for (int j = 0; j < numSlices; j++) {
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * j, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * j, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * (j + 1), radius);
			Vec2 uvBL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvBR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));

			if (j == numSlices - 1) {
				TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), 0.0f, radius);
				BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, 0.0f, radius);
				uvTR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
				uvBR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			}

			Vertex_PCUTBN blVertex(BL, (BL - center).GetNormalized(), color, uvBL);
			Vertex_PCUTBN trVertex(TR, (TR - center).GetNormalized(), color, uvTR);
			Vertex_PCUTBN tlVertex(TL, (TL - center).GetNormalized(), color, uvTL);
			Vertex_PCUTBN brVertex(BR, (BR - center).GetNormalized(), color, uvBR);

			FigureOutWhereThisVertBelongs(blVertex);
			FigureOutWhereThisVertBelongs(trVertex);
			FigureOutWhereThisVertBelongs(tlVertex);

			FigureOutWhereThisVertBelongs(trVertex);
			FigureOutWhereThisVertBelongs(blVertex);
			FigureOutWhereThisVertBelongs(brVertex);
		}
	}

	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& color, const AABB2& UVs)
{
	unsigned int indexOffset = (unsigned int)verts.size();
	std::vector<Vertex_PCU> tempVerts;

	//Lets first draw the top half of the sphere
	float latitudeDegrees = 180.0f / numStacks;
	float longtitudeDegrees = 360.0f / numSlices;

	std::map<Vertex_PCU, unsigned int> verticesToIndicesMap;
	auto FigureOutWhereThisVertBelongs = [&](const Vertex_PCU& vertex) {
		auto foundPair = verticesToIndicesMap.find(vertex);
		if (foundPair != verticesToIndicesMap.end()) {
			indices.push_back(foundPair->second);
		}
		else {
			unsigned int newIndex = indexOffset + static_cast<unsigned int>(tempVerts.size());
			verticesToIndicesMap[vertex] = newIndex;
			tempVerts.push_back(vertex);
			indices.push_back(newIndex);
		}
	};

	//1. edge case: i = 0 (Add bottom)
	Vertex_PCU bottom(center + Vec3(0.0f, 0.0f, -radius), color, UVs.m_mins);
	for (int j = 0; j < numSlices; j++) {
		Vec3 TL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees, longtitudeDegrees * j, radius);
		Vec3 TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees, longtitudeDegrees * (j + 1), radius);
		Vec2 uvTR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		if (j == numSlices - 1) {
			//To avoid float imprecision
			TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees, 0.0f, radius);
			uvTR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		}
		Vec2 uvTL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vertex_PCU trVertex(TR, color, uvTR);
		Vertex_PCU tlVertex(TL, color, uvTL);
		FigureOutWhereThisVertBelongs(bottom);
		FigureOutWhereThisVertBelongs(trVertex);
		FigureOutWhereThisVertBelongs(tlVertex);
	}

	//2. edge case: i = numStacks - 1 (Add top)
	Vertex_PCU top(center + Vec3(0.0f, 0.0f, radius), color, UVs.m_maxs);
	for (int j = 0; j < numSlices; j++) {
		Vec3 BL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (numStacks - 1), longtitudeDegrees * j, radius);
		Vec3 BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (numStacks - 1), longtitudeDegrees * (j + 1), radius);
		Vec2 uvBR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (numStacks - 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		if (j == numSlices - 1) {
			BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (numStacks - 1), 0.0f, radius);
			uvBR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (numStacks - 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		}
		Vec2 uvBL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (numStacks - 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vertex_PCU blVertex(BL, color, uvBL);
		Vertex_PCU brVertex(BR, color, uvBR);
		FigureOutWhereThisVertBelongs(top);
		FigureOutWhereThisVertBelongs(blVertex);
		FigureOutWhereThisVertBelongs(brVertex);
	}

	for (int i = 1; i < numStacks - 1; i++) {
		for (int j = 0; j < numSlices; j++) {
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * j, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * j, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), longtitudeDegrees * (j + 1), radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, longtitudeDegrees * (j + 1), radius);
			Vec2 uvBL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTL = Vec2(RangeMapClamped(longtitudeDegrees * j, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvTR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			Vec2 uvBR = Vec2(RangeMapClamped(longtitudeDegrees * (j + 1), 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));

			if (j == numSlices - 1) {
				TR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * (i + 1), 0.0f, radius);
				BR = center + Vec3::MakeFromPolarDegrees(-90.0f + latitudeDegrees * i, 0.0f, radius);
				uvTR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * (i + 1), -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
				uvBR = Vec2(RangeMapClamped(0.0f, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMapClamped(-90.0f + latitudeDegrees * i, -90.0f, 90.0f, UVs.m_mins.y, UVs.m_maxs.y));
			}

			Vertex_PCU blVertex(BL, color, uvBL);
			Vertex_PCU trVertex(TR, color, uvTR);
			Vertex_PCU tlVertex(TL, color, uvTL);
			Vertex_PCU brVertex(BR, color, uvBR);

			FigureOutWhereThisVertBelongs(blVertex);
			FigureOutWhereThisVertBelongs(trVertex);
			FigureOutWhereThisVertBelongs(tlVertex);

			FigureOutWhereThisVertBelongs(trVertex);
			FigureOutWhereThisVertBelongs(blVertex);
			FigureOutWhereThisVertBelongs(brVertex);
		}
	}

	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

void AddVertsForXYGrid3D(std::vector<Vertex_PCU>& verts, int gridSpan, int everyIntervalToHighlightLine)
{
	int numParallelLinesForEachAxis = gridSpan;
	float halfLineThickness = 0.0f;
	float halfGridSpan = float(gridSpan / 2);
	for (int i = 0; i < numParallelLinesForEachAxis; i++) {
		float startingPoint = float(-halfGridSpan + i);
		//If line passes origin, make it thicker and brighter
		if (i == (int)halfGridSpan) {
			halfLineThickness = 0.05f;
			AddVertsForAABB3D(verts, AABB3(Vec3(-halfGridSpan, startingPoint - halfLineThickness, -halfLineThickness), Vec3(halfGridSpan, startingPoint + halfLineThickness, halfLineThickness)), Rgba8::RED);
			AddVertsForAABB3D(verts, AABB3(Vec3(startingPoint - halfLineThickness, -halfGridSpan, -halfLineThickness), Vec3(startingPoint + halfLineThickness, halfGridSpan, halfLineThickness)), Rgba8::GREEN);
		}
		else if (i % everyIntervalToHighlightLine == 0) {
			halfLineThickness = 0.025f;
			AddVertsForAABB3D(verts, AABB3(Vec3(-halfGridSpan, startingPoint - halfLineThickness, -halfLineThickness), Vec3(halfGridSpan, startingPoint + halfLineThickness, halfLineThickness)), Rgba8(200, 0, 0));
			AddVertsForAABB3D(verts, AABB3(Vec3(startingPoint - halfLineThickness, -halfGridSpan, -halfLineThickness), Vec3(startingPoint + halfLineThickness, halfGridSpan, halfLineThickness)), Rgba8(0, 200, 0));
		}
		else {	//Just add the grey lines
			halfLineThickness = 0.0125f;
			AddVertsForAABB3D(verts, AABB3(Vec3(-halfGridSpan, startingPoint - halfLineThickness, -halfLineThickness), Vec3(halfGridSpan, startingPoint + halfLineThickness, halfLineThickness)), Rgba8::GREY);
			AddVertsForAABB3D(verts, AABB3(Vec3(startingPoint - halfLineThickness, -halfGridSpan, -halfLineThickness), Vec3(startingPoint + halfLineThickness, halfGridSpan, halfLineThickness)), Rgba8::GREY);
		}
	}
}

void AddVertsForConeZ3D(std::vector<Vertex_PCU>& verts, const Vec2& centerXY, const FloatRange& minmaxZ, float radius, const Rgba8& color, const AABB2& UVs, int numSlices)
{
	float degreesPerSlice = 360.f / numSlices;
	for (int sliceIndex = 0; sliceIndex < numSlices; sliceIndex++) {
		float cosSliceIndex = CosDegrees(degreesPerSlice * sliceIndex);
		float sinSliceIndex = SinDegrees(degreesPerSlice * sliceIndex);
		float cosSliceIndex_1 = CosDegrees(degreesPerSlice * (sliceIndex + 1));
		float sinSliceIndex_1 = SinDegrees(degreesPerSlice * (sliceIndex + 1));

		Vec3 rightBottom(centerXY.x + (radius * cosSliceIndex), centerXY.y + (radius * sinSliceIndex), minmaxZ.m_min);
		Vec3 leftBottom(centerXY.x + (radius * cosSliceIndex_1), centerXY.y + (radius * sinSliceIndex_1), minmaxZ.m_min);

		//Same calculation as the cylinder
		Vec2 rightTopUV = Vec2(RangeMap(cosSliceIndex, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vec2 leftTopUV = Vec2(RangeMap(cosSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vec2 rightBottomUV(rightTopUV.x, 1.0f - rightTopUV.y);
		Vec2 leftBottomUV(leftTopUV.x, 1.0f - leftTopUV.y);

		verts.push_back(Vertex_PCU(rightBottom, color, rightBottomUV));
		verts.push_back(Vertex_PCU(Vec3(centerXY, minmaxZ.m_min), color, UVs.GetCenter()));
		verts.push_back(Vertex_PCU(leftBottom, color, leftBottomUV));

		verts.push_back(Vertex_PCU(leftBottom, color, leftBottomUV));
		verts.push_back(Vertex_PCU(Vec3(centerXY, minmaxZ.m_max), color, UVs.GetCenter()));
		verts.push_back(Vertex_PCU(rightBottom, color, rightBottomUV));
	}
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& tint, const AABB2& UVs)
{
 	Vec3 fromStartToEnd = end - start;
	if (fromStartToEnd.x == 0.0f && fromStartToEnd.y == 0.0f) {
		AddVertsForCylinderZ3D(verts, Vec2(start.x, start.y), FloatRange(start.z, end.z), radius, numSlices, tint, UVs);
		return;
	}
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForCylinderZ3D(tempVerts, Vec2(), FloatRange(0.0f, fromStartToEnd.GetLength()), radius, numSlices, tint, UVs);
	Vec3 newUp = fromStartToEnd.GetNormalized();
	Vec3 newLeft = CrossProduct3D(Vec3(0.0f, 0.0f, 1.0f), newUp).GetNormalized();
	Vec3 newFwd = CrossProduct3D(newLeft, newUp).GetNormalized();
	TransformVertexArray3D(tempVerts, Mat44(newFwd, newLeft, newUp, start));
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color, const AABB2& UVs, int numSlices)
{
	Vec3 fromStartToEnd = end - start;
	if (fromStartToEnd.x == 0.0f && fromStartToEnd.y == 0.0f) {
		AddVertsForConeZ3D(verts, Vec2(start.x, start.y), FloatRange(start.z, end.z), radius, color, UVs, numSlices);
		return;
	}
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForConeZ3D(tempVerts, Vec2(), FloatRange(0.0f, fromStartToEnd.GetLength()), radius, color, UVs, numSlices);
	Vec3 newUp = fromStartToEnd.GetNormalized();
	Vec3 newLeft = CrossProduct3D(Vec3(0.0f, 0.0f, 1.0f), newUp).GetNormalized();
	Vec3 newFwd = CrossProduct3D(newLeft, newUp).GetNormalized();
	TransformVertexArray3D(tempVerts, Mat44(newFwd, newLeft, newUp, start));
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

void AddVertsForCircleZ3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs)
{
	float degreesPerSlice = 360.f / numSlices;
	for (int sliceIndex = 0; sliceIndex < numSlices; sliceIndex++) {
		float cosSliceIndex = CosDegrees(degreesPerSlice * sliceIndex);
		float sinSliceIndex = SinDegrees(degreesPerSlice * sliceIndex);
		float cosSliceIndex_1 = CosDegrees(degreesPerSlice * (sliceIndex + 1));
		float sinSliceIndex_1 = SinDegrees(degreesPerSlice * (sliceIndex + 1));

		Vec3 rightPos(center.x + (radius * cosSliceIndex), center.y + (radius * sinSliceIndex), center.z);
		Vec3 leftPos(center.x + (radius * cosSliceIndex_1), center.y + (radius * sinSliceIndex_1), center.z);

		Vec2 rightUV = Vec2(RangeMap(cosSliceIndex, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vec2 leftUV = Vec2(RangeMap(cosSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));

		verts.push_back(Vertex_PCU(leftPos, color, leftUV));
		verts.push_back(Vertex_PCU(center, color, UVs.GetCenter()));
		verts.push_back(Vertex_PCU(rightPos, color, rightUV));
	}
}

void AddVertsForCircle3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float radius, const Vec3& normal, const Rgba8& color, const AABB2& UVs)
{
	if (normal == Vec3(0.0f, 0.0f, 1.0f)) {
		AddVertsForCircleZ3D(verts, numSlices, center, radius, color, UVs);
		return;
	}
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForCircleZ3D(tempVerts, numSlices, Vec3(0.0f, 0.0f, 0.0f), radius, color, UVs);
	Vec3 newUp = normal;
	Vec3 newLeft = CrossProduct3D(Vec3(0.0f, 0.0f, 1.0f), newUp).GetNormalized();
	Vec3 newFwd = CrossProduct3D(newLeft, newUp).GetNormalized();
	TransformVertexArray3D(tempVerts, Mat44(newFwd, newLeft, newUp, center));
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

void AddVertsForRingZ3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float innerRadius, float outerRadius, const Rgba8& color, const AABB2& UVs)
{
	if (innerRadius >= outerRadius || innerRadius == 0.0f || outerRadius == 0.0f) {
		ERROR_AND_DIE("WRONG input for AddVertsForRingZ3D()");
	}
	int numTriangles = 2 * numSlices;
	int numVertices = 3 * numTriangles;
	float degreesPerSide = 360.f / static_cast<float> (numSlices);
	std::vector<Vertex_PCU> vertices;
	vertices.resize(numVertices);

	float innerOuterRadiusRatio = innerRadius / outerRadius;
	AABB2 smallUVBox = UVs.GetBoxWithin(Vec2(0.5f - innerOuterRadiusRatio * 0.5f, 0.5f - innerOuterRadiusRatio * 0.5f), Vec2(0.5f + innerOuterRadiusRatio * 0.5f, 0.5f + innerOuterRadiusRatio * 0.5f));

	for (int sideIndex = 0; sideIndex < numSlices; sideIndex++) {
		float cosSliceIndex = CosDegrees(degreesPerSide * sideIndex);
		float cosSliceIndex_1 = CosDegrees(degreesPerSide * (sideIndex + 1));
		float sinSliceIndex = SinDegrees(degreesPerSide * sideIndex);
		float sinSliceIndex_1 = SinDegrees(degreesPerSide * (sideIndex + 1));

		Vec3 I_L(center.x + (innerRadius * cosSliceIndex_1), center.y + (innerRadius * sinSliceIndex_1), 0.f);
		Vec3 I_R(center.x + (innerRadius * cosSliceIndex), center.y + (innerRadius * sinSliceIndex), 0.f);
		Vec2 rightInnerUV = Vec2(RangeMap(cosSliceIndex, -1.0f, 1.0f, smallUVBox.m_mins.x, smallUVBox.m_maxs.x), RangeMap(sinSliceIndex, -1.0f, 1.0f, smallUVBox.m_mins.y, smallUVBox.m_maxs.y));
		Vec2 leftInnerUV = Vec2(RangeMap(cosSliceIndex_1, -1.0f, 1.0f, smallUVBox.m_mins.x, smallUVBox.m_maxs.x), RangeMap(sinSliceIndex_1, -1.0f, 1.0f, smallUVBox.m_mins.y, smallUVBox.m_maxs.y));

		Vec3 O_R(center.x + (outerRadius * cosSliceIndex), center.y + (outerRadius * sinSliceIndex), 0.f);
		Vec3 O_L(center.x + (outerRadius * cosSliceIndex_1), center.y + (outerRadius * sinSliceIndex_1), 0.f);
		Vec2 rightOuterUV = Vec2(RangeMap(cosSliceIndex, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));
		Vec2 leftOuterUV = Vec2(RangeMap(cosSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x), RangeMap(sinSliceIndex_1, -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y));

		vertices[6 * sideIndex + 0].m_position = I_L;
		vertices[6 * sideIndex + 1].m_position = I_R;
		vertices[6 * sideIndex + 2].m_position = O_R;

		vertices[6 * sideIndex + 3].m_position = I_L;
		vertices[6 * sideIndex + 4].m_position = O_R;
		vertices[6 * sideIndex + 5].m_position = O_L;

		vertices[6 * sideIndex + 0].m_color = color;
		vertices[6 * sideIndex + 1].m_color = color;
		vertices[6 * sideIndex + 2].m_color = color;

		vertices[6 * sideIndex + 3].m_color = color;
		vertices[6 * sideIndex + 4].m_color = color;
		vertices[6 * sideIndex + 5].m_color = color;

		vertices[6 * sideIndex + 0].m_uvTexCoords = leftInnerUV;
		vertices[6 * sideIndex + 1].m_uvTexCoords = rightInnerUV;
		vertices[6 * sideIndex + 2].m_uvTexCoords = rightOuterUV;

		vertices[6 * sideIndex + 3].m_uvTexCoords = leftInnerUV;
		vertices[6 * sideIndex + 4].m_uvTexCoords = rightOuterUV;
		vertices[6 * sideIndex + 5].m_uvTexCoords = leftOuterUV;
	}
	verts.insert(verts.end(), vertices.begin(), vertices.end());
}

void AddVertsForRing3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float innerRadius, float outerRadius, const Vec3& normal, const Rgba8& color, const AABB2& UVs)
{
	if (normal == Vec3(0.0f, 0.0f, 1.0f)) {
		AddVertsForRingZ3D(verts, numSlices, center, innerRadius, outerRadius, color, UVs);
		return;
	}
	std::vector<Vertex_PCU> tempVerts;
	AddVertsForRingZ3D(tempVerts, numSlices, Vec3(0.0f, 0.0f, 0.0f), innerRadius, outerRadius, color, UVs);
	Vec3 newUp = normal;
	Vec3 newLeft = CrossProduct3D(Vec3(0.0f, 0.0f, 1.0f), newUp).GetNormalized();
	Vec3 newFwd = CrossProduct3D(newLeft, newUp).GetNormalized();
	TransformVertexArray3D(tempVerts, Mat44(newFwd, newLeft, newUp, center));
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
}

void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& tint)
{
	Vec3 fromStartToEnd = end - start;
	AddVertsForCylinder3D(verts, start, start + fromStartToEnd * 0.8f, radius, numSlices, tint);
	AddVertsForCone3D(verts, start + fromStartToEnd * 0.8f, end, radius * 2.0f, tint, AABB2::ZERO_TO_ONE, numSlices);
}

void AddVertsForWireframeAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, float lineThickness, const Rgba8& color)
{
	//Back/Front_Right/Left_Bottom/Top (Think of where your eye is)
	Vec3 vFRB(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 vFRT(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 vFLB(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 vFLT(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 vBRB(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 vBRT(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 vBLB(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 vBLT(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForLineSegment3D(verts, vFRB, vFRT, lineThickness, color);
	AddVertsForLineSegment3D(verts, vFLB, vFLT, lineThickness, color);
	AddVertsForLineSegment3D(verts, vBRB, vBRT, lineThickness, color);
	AddVertsForLineSegment3D(verts, vBLB, vBLT, lineThickness, color);

	AddVertsForLineSegment3D(verts, vFLB, vFRB, lineThickness, color);
	AddVertsForLineSegment3D(verts, vBLB, vBRB, lineThickness, color);
	AddVertsForLineSegment3D(verts, vBLT, vBRT, lineThickness, color);
	AddVertsForLineSegment3D(verts, vFLT, vFRT, lineThickness, color);

	AddVertsForLineSegment3D(verts, vBLB, vFLB, lineThickness, color);
	AddVertsForLineSegment3D(verts, vBRB, vFRB, lineThickness, color);
	AddVertsForLineSegment3D(verts, vBRT, vFRT, lineThickness, color);
	AddVertsForLineSegment3D(verts, vBLT, vFLT, lineThickness, color);
}

void AddVertsForTorusZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, unsigned int minorSegments, unsigned int majorSegments, float minorRadius, float majorRadius, const Vec3& centerPos, const Rgba8& color, const AABB2& UVs)
{
	if (minorRadius >= majorRadius) {
		ERROR_AND_DIE("minor radius >= major radius for AddVertsForTorus()");
	}

	if (minorRadius < 0 || majorRadius < 0) {
		ERROR_AND_DIE("Invalid radius parameter in AddVertsForTorus()");
	}

	if (minorSegments == 0 || majorSegments == 0) {
		ERROR_AND_DIE("minorSegments or majorSegments is 0");
	}

	unsigned int indexOffset = (unsigned int)verts.size();

	std::vector<Vertex_PCU> tempVerts;

	// Generate torus vertices
	for (unsigned int i = 0; i <= majorSegments; ++i)
	{
		float majorAngle = 360.0f * static_cast<float>(i) / majorSegments;
		for (unsigned int j = 0; j <= minorSegments; ++j)
		{
			float minorAngle = 360.0f * static_cast<float>(j) / minorSegments;
			float x = centerPos.x + (majorRadius + minorRadius * CosDegrees(minorAngle)) * CosDegrees(majorAngle);
			float y = centerPos.y + (majorRadius + minorRadius * CosDegrees(minorAngle)) * SinDegrees(majorAngle);
			float z = centerPos.z + minorRadius * SinDegrees(minorAngle);

			Vertex_PCU vertex;
			vertex.m_position = Vec3(x, y, z);
			vertex.m_color = color;
			vertex.m_uvTexCoords = UVs.GetPointAtUV(Vec2(static_cast<float>(i) / majorSegments, static_cast<float>(j) / minorSegments));

			tempVerts.push_back(vertex);
		}
	}

	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());

	// Generate torus indices
	for (unsigned int i = 0; i < majorSegments; ++i)
	{
		for (unsigned int j = 0; j < minorSegments; ++j)
		{
			int index0 = i * (minorSegments + 1) + j;
			int index1 = index0 + 1;
			int index2 = (i + 1) * (minorSegments + 1) + j;
			int index3 = index2 + 1;

			// Triangle 1
			indices.push_back(indexOffset + index0);
			indices.push_back(indexOffset + index1);
			indices.push_back(indexOffset + index2);

			// Triangle 2
			indices.push_back(indexOffset + index2);
			indices.push_back(indexOffset + index1);
			indices.push_back(indexOffset + index3);
		}
	}
}

void AddVertsForTorus3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, unsigned int minorSegments, unsigned int majorSegments, float minorRadius, float majorRadius, const Vec3& centerPos, Vec3 normal, const Rgba8& color, const AABB2& UVs)
{
	if (normal == Vec3(0.0f, 0.0f, 1.0f)) {
		AddVertsForTorusZ3D(verts, indices, minorSegments, majorSegments, minorRadius, majorRadius, centerPos, color, UVs);
		return;
	}

	bool isIndicesEmpty = true;
	if (indices.size() != 0) {
		isIndicesEmpty = false;
	}

	std::vector<Vertex_PCU> tempVerts;
	std::vector<unsigned int> tempIndices;

	AddVertsForTorusZ3D(tempVerts, tempIndices, minorSegments, majorSegments, minorRadius, majorRadius, centerPos, color, UVs);
	
	Vec3 newUp = normal;
	Vec3 newLeft = CrossProduct3D(Vec3(0.0f, 0.0f, 1.0f), newUp).GetNormalized();
	Vec3 newFwd = CrossProduct3D(newLeft, newUp).GetNormalized();
	TransformVertexArray3D(tempVerts, Mat44(newFwd, newLeft, newUp, centerPos));

	if (isIndicesEmpty == false) {
		unsigned int numVertices = (unsigned int)verts.size();
		for (int i = 0; i < indices.size(); i++) {
			indices[i] += numVertices;
		}
	}

	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());
	indices.insert(indices.end(), tempIndices.begin(), tempIndices.end());
}

void AddVertsForHexZ3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& center, float innerRadius, float outerRadius, const Rgba8& color)
{
	std::vector<Vertex_PCUTBN> newVerts;

	constexpr int NUM_SIDES = 6;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTICES = 3 * NUM_TRIS;
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float> (NUM_SIDES);

	newVerts.reserve((size_t)NUM_VERTICES);

	for (unsigned int sideIndex = 0; sideIndex < NUM_SIDES; sideIndex++) {
		Vec3 I_L(center.x + (innerRadius * CosDegrees(DEGREES_PER_SIDE * (sideIndex + 1))), 
			center.y + (innerRadius * SinDegrees(DEGREES_PER_SIDE * (sideIndex + 1))), center.z);
		Vec3 I_R(center.x + (innerRadius * CosDegrees(DEGREES_PER_SIDE * sideIndex)), 
			center.y + (innerRadius * SinDegrees(DEGREES_PER_SIDE * sideIndex)), center.z);
		Vec3 O_R(center.x + (outerRadius * CosDegrees(DEGREES_PER_SIDE * sideIndex)), 
			center.y + (outerRadius * SinDegrees(DEGREES_PER_SIDE * sideIndex)), center.z);
		Vec3 O_L(center.x + (outerRadius * CosDegrees(DEGREES_PER_SIDE * (sideIndex + 1))), 
			center.y + (outerRadius * SinDegrees(DEGREES_PER_SIDE * (sideIndex + 1))), center.z);

		size_t indicesOffset = verts.size() + newVerts.size();

		newVerts.push_back(Vertex_PCUTBN(I_L, Vec3(0.0f, 0.0f, 1.0f), color));
		newVerts.push_back(Vertex_PCUTBN(I_R, Vec3(0.0f, 0.0f, 1.0f), color));
		newVerts.push_back(Vertex_PCUTBN(O_R, Vec3(0.0f, 0.0f, 1.0f), color));
		newVerts.push_back(Vertex_PCUTBN(O_L, Vec3(0.0f, 0.0f, 1.0f), color));

		indices.push_back((unsigned int)indicesOffset + 0);
		indices.push_back((unsigned int)indicesOffset + 1);
		indices.push_back((unsigned int)indicesOffset + 2);

		indices.push_back((unsigned int)indicesOffset + 0);
		indices.push_back((unsigned int)indicesOffset + 2);
		indices.push_back((unsigned int)indicesOffset + 3);
	}

	verts.insert(verts.end(), newVerts.begin(), newVerts.end());
}

void CalculateTangents(std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices)
{
	size_t vertexCount = verts.size();
	std::vector<Vec3> tangents;
	tangents.resize(vertexCount, Vec3());
	std::vector<Vec3> bitangents;
	bitangents.resize(vertexCount, Vec3());
	/*
	std::vector<Vec3> normals;
	normals.resize(vertexCount, Vec3());
	*/

	for (size_t i = 0; i < indices.size(); i+=3) {
		int i0 = indices[i];
		int i1 = indices[i + 1];
		int i2 = indices[i + 2];
		const Vec3& p0 = verts[i0].m_position;
		const Vec3& p1 = verts[i1].m_position;
		const Vec3& p2 = verts[i2].m_position;
		const Vec2& uv0 = verts[i0].m_uvTexCoords;
		const Vec2& uv1 = verts[i1].m_uvTexCoords;
		const Vec2& uv2 = verts[i2].m_uvTexCoords;

		Vec3 e1 = p1 - p0;
		Vec3 e2 = p2 - p0;

		float x1 = uv1.x - uv0.x;
		float x2 = uv2.x - uv0.x;
		float y1 = uv1.y - uv0.y;
		float y2 = uv2.y - uv0.y;

		float r = 1.0f / (x1 * y2 - x2 * y1);
		Vec3 t = (e1 * y2 - e2 * y1) * r;
		Vec3 b = (e2 * x1 - e1 * x2) * r;

		tangents[i0] += t;
		tangents[i1] += t;
		tangents[i2] += t;
		bitangents[i0] += b;
		bitangents[i1] += b;
		bitangents[i2] += b;

		/*
		//Now calculate the normal
		Vec3 n = CrossProduct3D(e1, e2);
		normals[i0] += n;
		normals[i1] += n;
		normals[i2] += n;
		*/
	}

	for (int i = 0; i < vertexCount; i++) {
		//normals[i] = normals[i].GetNormalized();
		tangents[i] = (tangents[i] - DotProduct3D(tangents[i], verts[i].m_normal) * verts[i].m_normal).GetNormalized();
		bitangents[i] = (bitangents[i] - DotProduct3D(bitangents[i], verts[i].m_normal) * verts[i].m_normal - DotProduct3D(bitangents[i], tangents[i]) * tangents[i]).GetNormalized();

		verts[i].m_tangent = tangents[i];
		verts[i].m_binormal = bitangents[i];
	}
}

void CalculateAveragedNormals(std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices)
{
	size_t vertexCount = verts.size();
	std::vector<Vec3> normals;
	normals.resize(vertexCount, Vec3());

	for (size_t i = 0; i < indices.size(); i += 3) {
		int i0 = indices[i];
		int i1 = indices[i + 1];
		int i2 = indices[i + 2];
		const Vec3& p0 = verts[i0].m_position;
		const Vec3& p1 = verts[i1].m_position;
		const Vec3& p2 = verts[i2].m_position;

		Vec3 e1 = p1 - p0;
		Vec3 e2 = p2 - p0;

		//Now calculate the normal
		Vec3 n = CrossProduct3D(e1, e2);
		normals[i0] += n;
		normals[i1] += n;
		normals[i2] += n;
	}

	for (int i = 0; i < vertexCount; i++) {
		verts[i].m_normal = normals[i].GetNormalized();
	}
}
