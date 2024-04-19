#include "Engine/Mesh/MeshOperationUtils.hpp"
#include "Engine/Mesh/HalfEdge.hpp"
#include "Engine/Mesh/Face.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

/*
std::vector<ControlPoint> GetOneRingNeighbor(const ControlPoint& controlPoint)
{
	std::vector<ControlPoint> out_oneRingNeighbor;

	const HalfEdge* const originalHE = controlPoint.m_outgoingHE;
	const HalfEdge* currentHE = originalHE;

	do {
		out_oneRingNeighbor.push_back(*currentHE->m_opposingHE->m_originatingCP);
		currentHE = currentHE->m_opposingHE->m_nextHE;
	} while (currentHE != originalHE);

	return out_oneRingNeighbor;
}

bool AreFacesAndHalfEdgesValid(const std::vector<Face*>& faces, const std::vector<HalfEdge*>& halfEdges)
{
	for (int i = 0; i < faces.size(); i++) {
		if (faces[i] == nullptr || faces[i]->m_he == nullptr)
			return false;
	}
	int boundaryHalfEdgeCounter = 0;
	for (int i = 0; i < halfEdges.size(); i++) {
		if (halfEdges[i] == nullptr || halfEdges[i]->m_nextHE == nullptr || halfEdges[i]->m_opposingHE == nullptr || halfEdges[i]->m_originatingCP == nullptr)
			return false;
		if (halfEdges[i]->m_leftSideFace == nullptr)
			boundaryHalfEdgeCounter++;
	}
	return true;
}
*/