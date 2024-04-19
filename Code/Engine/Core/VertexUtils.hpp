#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>

struct OBB2;
struct Capsule2;
struct LineSegment2;
struct CubicBezierCurve2;
struct CubicHermiteCurve2;
struct CatmullRomSpline2;
struct ConvexPoly2D;

void TransformVertexArrayXY3D(const int num_vertices, Vertex_PCU* const tempWorldVerts, float scaleXY, const float rotationDegreesZ, const Vec2& translateXY);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform);
AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU>& verts);

void AddVertsForAABB2(std::vector<Vertex_PCU>& verts, const AABB2& bounds, const Rgba8& color = Rgba8(255, 255, 255, 255), const AABB2& uvs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForWireframeAABB2(std::vector<Vertex_PCU>& verts, const AABB2& bounds, float thickness, const Rgba8& color = Rgba8(255, 255, 255, 255));
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color, const AABB2& uvs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Capsule2 const& capsule, Rgba8 const& color);
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec2& center, float innerRadius, float outerRadius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, LineSegment2 const& lineSegment, float thickness, Rgba8 const& color);
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color);
void AddVertsForCubicBezierCurve2D(std::vector<Vertex_PCU>& verts, const CubicBezierCurve2& cubicBezierCurve, int subdivisions, float lineThickness, const Rgba8& color);
void AddVertsForCubicHermiteCurve2D(std::vector<Vertex_PCU>& verts, const CubicHermiteCurve2& cubicBezierCurve, int subdivisions, float lineThickness, const Rgba8& color);
void AddVertsForCatmullRomSpline2D(std::vector<Vertex_PCU>& verts, const CatmullRomSpline2& catmullRomSpline, int subdivisions, float lineThickness, const Rgba8& color);
void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, const ConvexPoly2D& convexPoly, const Rgba8& color);

void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float thickness, const Rgba8& color);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3 topLeft, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3 topLeft, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& topLeft, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForWireframeQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3 topLeft, float thickness, const Rgba8& color = Rgba8::WHITE);

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, const Vec2& centerXY, FloatRange const& minmaxZ, float radius, int numSlices, Rgba8 const& tint = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& tint = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numLatitudeSlices = 8);
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& tint = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphere3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, const Vec3& center, float radius, int numSlices, int numStacks, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForXYGrid3D(std::vector<Vertex_PCU>& verts, int gridSpan = 100, int everyIntervalToHighlightLine = 5);
void AddVertsForConeZ3D(std::vector<Vertex_PCU>& verts, const Vec2& centerXY, const FloatRange& minmaxZ, float radius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8);
void AddVertsForCircleZ3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float radius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCircle3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float radius, const Vec3& normal, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRingZ3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float innerRadius, float outerRadius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRing3D(std::vector<Vertex_PCU>& verts, int numSlices, const Vec3& center, float innerRadius, float outerRadius, const Vec3& normal, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, int numSlices, const Rgba8& tint = Rgba8::WHITE);

void AddVertsForWireframeAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, float lineThickness, const Rgba8& color = Rgba8::WHITE);

void AddVertsForTorusZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, unsigned int minorSegments, unsigned int majorSegments,float minorRadius, float majorRadius, const Vec3& centerPos, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForTorus3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indices, unsigned int minorSegments, unsigned int majorSegments, float minorRadius, float majorRadius, const Vec3& centerPos, Vec3 normal, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForHexZ3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indices, const Vec3& center, float innerRadius, float outerRadius, const Rgba8& color);

void CalculateTangents(std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices);
void CalculateAveragedNormals(std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices);