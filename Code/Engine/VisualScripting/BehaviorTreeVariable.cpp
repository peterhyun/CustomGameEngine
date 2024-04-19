#include "Engine/VisualScripting/BehaviorTreeVariable.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"

BehaviorTreeVariable::BehaviorTreeVariable(const std::string& nodeDisplayStr, bool isRootNode, bool overrideDefaultSettings, const Vec2& overridenDim) : m_nodeDisplayStr(nodeDisplayStr), m_isRootNode(isRootNode)
{
	if (overrideDefaultSettings) {
		m_dimensions = overridenDim;
	}
	else {
		m_dimensions = Vec2((float)TREENODE_DIMX, (float)TREENODE_DIMY);
	}
}

BehaviorTreeVariable::~BehaviorTreeVariable()	//You need implementation for a pure virtual destructor, even if it seems counter intuitive
{
}

void BehaviorTreeVariable::MoveNode(const Vec2& newPos)
{
	m_posInEditor = newPos;
}

std::string BehaviorTreeVariable::GetNodeDisplayStr() const
{
	return m_nodeDisplayStr;
}

Vec2 BehaviorTreeVariable::GetCurrentPos() const
{
	return m_posInEditor;
}

const std::list<BehaviorTreeVariable*>& BehaviorTreeVariable::GetChildNodes() const
{
	return m_childNodes;
}

void BehaviorTreeVariable::AddOutlineVertsForMe(const Vec2& cursorPosInEditor, std::vector<Vertex_PCU>& out_verts, bool isHighlighted) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - TREENODE_DIMY, m_posInEditor.x + TREENODE_DIMX, m_posInEditor.y);
	if (isHighlighted) {
		AddVertsForAABB2(out_verts, AABB2(treeNodeAABB.m_mins - Vec2(0.7f, 0.7f), treeNodeAABB.m_maxs + Vec2(0.7f, 0.7f)), Rgba8::PASTEL_PINK);
	}
	AddVertsForAABB2(out_verts, treeNodeAABB.GetBoxWithin(Vec2(0.0f, 0.8f), Vec2(1.0f, 1.0f)), Rgba8::CHELSEA_GREY);
	AddVertsForAABB2(out_verts, treeNodeAABB.GetBoxWithin(Vec2(0.0f, 0.0f), Vec2(1.0f, 0.8f)), Rgba8::BLACK);

	Rgba8 topPortOutlineColor = Rgba8::YELLOW;
	Rgba8 bottomPortOutlineColor = Rgba8::YELLOW;

	Vec2 topPortPos;
	float topPortRadius = 0.0f;
	Vec2 bottomPortPos;
	float bottomPortRadius = 0.0f;
	GetTopPortDiscData(topPortPos, topPortRadius);
	GetBottomPortDiscData(bottomPortPos, bottomPortRadius);
	if (IsPointInsideDisc2D(cursorPosInEditor, topPortPos, topPortRadius)) {
		topPortOutlineColor = Rgba8::BLUE;
	}
	if (IsPointInsideDisc2D(cursorPosInEditor, bottomPortPos, bottomPortRadius)) {
		bottomPortOutlineColor = Rgba8::BLUE;
	}

	if (m_isRootNode == false) {
		AddVertsForDisc2D(out_verts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 1.0f)), TREENODE_PORTRADIUS, topPortOutlineColor);
		AddVertsForDisc2D(out_verts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 1.0f)), TREENODE_PORTRADIUS * 0.75f, Rgba8::BLACK);
	}
	AddVertsForDisc2D(out_verts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 0.0f)), TREENODE_PORTRADIUS, bottomPortOutlineColor);
	AddVertsForDisc2D(out_verts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 0.0f)), TREENODE_PORTRADIUS * 0.75f, Rgba8::BLACK);

	//Add the lines now
	for (auto childNode : m_childNodes) {
		if (childNode) {
			Vec2 childTopPortPos;
			float childTopPortRadius = 0.0f;
			childNode->GetTopPortDiscData(childTopPortPos, childTopPortRadius);
			AddVertsForLineSegment2D(out_verts, bottomPortPos, childTopPortPos, TREEEDITOR_CONNECTIONLINETHICKNESS, Rgba8::WHITE);
		}
	}
}

void BehaviorTreeVariable::AddTextVertsForMe(const BitmapFont& bitmapFont, std::vector<Vertex_PCU>& out_verts) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - TREENODE_DIMY, m_posInEditor.x + TREENODE_DIMX, m_posInEditor.y);
	AABB2 titleBarAABB = treeNodeAABB.GetBoxWithin(Vec2(0.0f, 0.8f), Vec2(1.0f, 1.0f));
	bitmapFont.AddVertsForTextInBox2D(out_verts, titleBarAABB, 30.0f, m_nodeDisplayStr, Rgba8::WHITE, FONT_ASPECT);
}

void BehaviorTreeVariable::AddNodeToChildren(BehaviorTreeVariable& treeNode)
{
	m_childNodes.push_back(&treeNode);
	treeNode.m_parentNode = this;
}

BehaviorTreeVariable* BehaviorTreeVariable::RemoveNodeFromChildren(BehaviorTreeVariable& treeNode)
{
	bool isNodeWithinChildrenList = false;
	for (auto childNode : m_childNodes) {
		if (childNode == &treeNode) {
			isNodeWithinChildrenList = true;
			break;
		}
	}
	GUARANTEE_OR_DIE(isNodeWithinChildrenList == true, "treeNode NOT in the children list");
	m_childNodes.remove(&treeNode);
	treeNode.m_parentNode = nullptr;
	return &treeNode;
}

void BehaviorTreeVariable::GetTopPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - TREENODE_DIMY, m_posInEditor.x + TREENODE_DIMX, m_posInEditor.y);
	out_portDiscCenter = treeNodeAABB.GetPointAtUV(Vec2(0.5f, 1.0f));
	out_discRadius = TREENODE_PORTRADIUS;
}

void BehaviorTreeVariable::GetBottomPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - TREENODE_DIMY, m_posInEditor.x + TREENODE_DIMX, m_posInEditor.y);
	out_portDiscCenter = treeNodeAABB.GetPointAtUV(Vec2(0.5f, 0.0f));
	out_discRadius = TREENODE_PORTRADIUS;
}

bool BehaviorTreeVariable::IsRootNode() const
{
	return m_isRootNode;
}

bool BehaviorTreeVariable::IsCursorInside(const Vec2& cursorPos) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - TREENODE_DIMY, m_posInEditor.x + TREENODE_DIMX, m_posInEditor.y);
	return IsPointInsideAABB2D(cursorPos, treeNodeAABB);
}

bool BehaviorTreeVariable::IsDeletable() const
{
	return m_isDeletable;
}

bool BehaviorTreeVariable::IsMovable() const
{
	return m_isMovable;
}

bool BehaviorTreeVariable::IsNodeAChild(const BehaviorTreeVariable& childCandidate) const
{
	for (auto node : m_childNodes) {
		if (node == &childCandidate) {
			return true;
		}
	}
	return false;
}

BehaviorTreeVariable* BehaviorTreeVariable::GetParentNode() const
{
	return m_parentNode;
}

IntRange BehaviorTreeVariable::GetChildNumRange() const
{
	return m_childNumRange;
}

int BehaviorTreeVariable::GetNumChildNodes() const
{
	return (int)m_childNodes.size();
}

std::map<std::string, std::string> BehaviorTreeVariable::GetAndSetXMLAttributes()
{
	m_attributeKeyValues["position"] = Stringf("%.2f, %.2f", m_posInEditor.x, m_posInEditor.y);
	return m_attributeKeyValues;
}
