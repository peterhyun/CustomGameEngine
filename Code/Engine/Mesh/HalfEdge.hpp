#pragma once

struct ControlPoint;
struct Face;

struct HalfEdge {
public:
	ControlPoint* m_originatingCP = nullptr;
	HalfEdge* m_opposingHE = nullptr;
	Face* m_leftSideFace = nullptr;
	HalfEdge* m_nextHE = nullptr;
};