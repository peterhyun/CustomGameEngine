#include "Engine/VisualScripting/NodeHelperComponent.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

NodeHelperComponent::NodeHelperComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs)
	: m_ownerNode(ownerNode), m_nodeUVMins(nodeUVMins), m_nodeUVMaxs(nodeUVMaxs)
{
	AABB2 ownerAABB = m_ownerNode.GetAABB2();
	Vec2 initialMins = ownerAABB.GetPointAtUV(nodeUVMins);
	Vec2 initialMaxs = ownerAABB.GetPointAtUV(nodeUVMaxs);
	m_initialOffsetFromTopLeft = ownerAABB.GetPointAtUV(Vec2(nodeUVMins.x, nodeUVMaxs.y)) - ownerAABB.GetTopLeft();
	m_initialDimensions = initialMaxs - initialMins;
}

NodeHelperComponent::~NodeHelperComponent()
{
	delete m_textVBO;
	delete m_outlineVBO;
}

void NodeHelperComponent::RecalculateUVsToPreserveInitialDims(const Vec2& newNodeDims)
{
	AABB2 currentOwnerAABB = m_ownerNode.GetAABB2();
	DebuggerPrintf("Node: %s (AABB: m_mins: %.3f, %.3f/ m_maxs: %.3f, %.3f)\n"
		, m_ownerNode.GetNodeDisplayStr().c_str(), currentOwnerAABB.m_mins.x, currentOwnerAABB.m_mins.y, currentOwnerAABB.m_maxs.x, currentOwnerAABB.m_maxs.y);
	Vec2 topLeftPoint = currentOwnerAABB.GetTopLeft() + m_initialOffsetFromTopLeft;
	Vec2 bottomRightPoint(topLeftPoint.x + m_initialDimensions.x, topLeftPoint.y - m_initialDimensions.y);
	Vec2 currentOwnerAABBTopLeft = currentOwnerAABB.GetTopLeft();

	AABB2 newComponentAABB = AABB2(Vec2(topLeftPoint.x, bottomRightPoint.y), Vec2(bottomRightPoint.x, topLeftPoint.y));
	AABB2 futureOwnerAABB = AABB2(Vec2(currentOwnerAABBTopLeft.x, currentOwnerAABBTopLeft.y - newNodeDims.y), Vec2(currentOwnerAABBTopLeft.x + newNodeDims.x, currentOwnerAABBTopLeft.y));

	GetUVOfOneAABB2RelativeToAnother(newComponentAABB, futureOwnerAABB, m_nodeUVMins, m_nodeUVMaxs);
}

void NodeHelperComponent::RenderInEditor() const
{
	Renderer& renderer = m_ownerNode.GetConstPtrToEditor()->GetRefToRenderer();
	if (m_outlineVBO) {
		renderer.BindShader(nullptr);
		renderer.BindTexture(nullptr);
		renderer.DrawVertexBuffer(m_outlineVBO, (int)m_outlineVerts.size());
	}

	if (m_textVBO) {
		BitmapFont* bitmapFont = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		GUARANTEE_OR_DIE(bitmapFont != nullptr, "Check BitmapFont name");
		renderer.BindTexture(&bitmapFont->GetTexture());
		renderer.DrawVertexBuffer(m_textVBO, (int)m_textVerts.size());
	}
}

bool NodeHelperComponent::IsActive() const
{
	return m_isActive;
}
