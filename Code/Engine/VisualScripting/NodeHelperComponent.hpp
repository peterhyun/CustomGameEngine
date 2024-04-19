#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class BehaviorTreeNode;
class VertexBuffer;

class NodeHelperComponent {
public:
	NodeHelperComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs);
	virtual ~NodeHelperComponent();
	//This function assumes that the top left of the node is fixed no matter what dimension!
	void RecalculateUVsToPreserveInitialDims(const Vec2& newNodeDims);
	virtual void UpdateInEditor() = 0;
	virtual void RenderInEditor() const final;

	bool IsActive() const;

private:
	virtual void UpdateOutlineVertsForMe() = 0;
	virtual void UpdateTextVertsForMe() = 0;

protected:
	BehaviorTreeNode& m_ownerNode;
	Vec2 m_initialOffsetFromTopLeft;
	Vec2 m_initialDimensions;
	Vec2 m_nodeUVMins;
	Vec2 m_nodeUVMaxs;

	VertexBuffer* m_outlineVBO = nullptr;
	std::vector<Vertex_PCU> m_outlineVerts;

	VertexBuffer* m_textVBO = nullptr;
	std::vector<Vertex_PCU> m_textVerts;

	bool m_isActive = false;
};