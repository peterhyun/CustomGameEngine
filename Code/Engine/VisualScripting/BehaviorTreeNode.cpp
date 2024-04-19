#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/VisualScripting/NodeHelperComponent.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"

BehaviorTreeNode::BehaviorTreeNode(const std::string& nodeDisplayStr, bool isRootNode, bool overrideDefaultSettings, const Vec2& overridenDim) : m_nodeDisplayStr(nodeDisplayStr), m_isRootNode(isRootNode)
{
	if (overrideDefaultSettings) {
		m_dimensions = overridenDim;
	}
	else {
		m_dimensions = Vec2((float)TREENODE_DIMX, (float)TREENODE_DIMY);
	}
	m_initialDimensions = m_dimensions;
	AABB2 treeNodeAABB(0.0f, - m_dimensions.y, m_dimensions.x, 0.0f);
	m_initialTitleOutlineDimsY = treeNodeAABB.GetBoxWithin(Vec2(0.0f, 0.8f), Vec2(1.0f, 1.0f)).GetDimensions().y;
}

BehaviorTreeNode::~BehaviorTreeNode()	//You need implementation for a pure virtual destructor, even if it seems counter intuitive
{
	for (auto helperComponent : m_helperComponents) {
		delete helperComponent;
	}
	delete m_outlineVBO;
	delete m_textVBO;
	delete m_tickFlowVBO;
	
	delete m_tickFlowStopwatch;
}

bool BehaviorTreeNode::CheckSetupValidity()
{
	m_isSetupValidWhenLatestValidityCheck = m_childNumRange.IsOnRange((int)m_childNodes.size());
	return m_isSetupValidWhenLatestValidityCheck;
}

void BehaviorTreeNode::FillInMissingInformationFromXmlElement(const XmlElement& element)
{
	element;
}

void BehaviorTreeNode::MoveNode(const Vec2& newPos)
{
	m_posInEditor = newPos;
}

std::string BehaviorTreeNode::GetNodeDisplayStr() const
{
	return m_nodeDisplayStr;
}

Vec2 BehaviorTreeNode::GetCurrentPos() const
{
	return m_posInEditor;
}

const std::list<BehaviorTreeNode*>& BehaviorTreeNode::GetChildNodes() const
{
	return m_childNodes;
}

void BehaviorTreeNode::UpdateOutlineVertsForMe(const Vec2& cursorPosInEditor, bool isHighlighted)
{
	m_outlineVerts.clear();
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - m_dimensions.y, m_posInEditor.x + m_dimensions.x, m_posInEditor.y);
	if (m_isSetupValidWhenLatestValidityCheck == false) {
		AddVertsForAABB2(m_outlineVerts, AABB2(treeNodeAABB.m_mins - Vec2(1.0f, 1.0f), treeNodeAABB.m_maxs + Vec2(1.0f, 1.0f)), Rgba8::RED);
	}
	if (isHighlighted) {
		AddVertsForAABB2(m_outlineVerts, AABB2(treeNodeAABB.m_mins - Vec2(0.7f, 0.7f), treeNodeAABB.m_maxs + Vec2(0.7f, 0.7f)), Rgba8::PASTEL_PINK);
	}
	AddVertsForAABB2(m_outlineVerts, treeNodeAABB, Rgba8::BLACK);
	Vec2 topLeftPoint = treeNodeAABB.GetTopLeft();
	AddVertsForAABB2(m_outlineVerts, AABB2(Vec2(treeNodeAABB.m_mins.x, treeNodeAABB.m_maxs.y - m_initialTitleOutlineDimsY), treeNodeAABB.m_maxs), Rgba8::CHELSEA_GREY);

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
		AddVertsForDisc2D(m_outlineVerts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 1.0f)), TREENODE_PORTRADIUS, topPortOutlineColor);
		AddVertsForDisc2D(m_outlineVerts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 1.0f)), TREENODE_PORTRADIUS * 0.75f, Rgba8::BLACK);
	}
	if (!(m_childNumRange.m_min == 0 && m_childNumRange.m_max == 0)) {
		AddVertsForDisc2D(m_outlineVerts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 0.0f)), TREENODE_PORTRADIUS, bottomPortOutlineColor);
		AddVertsForDisc2D(m_outlineVerts, treeNodeAABB.GetPointAtUV(Vec2(0.5f, 0.0f)), TREENODE_PORTRADIUS * 0.75f, Rgba8::BLACK);
	}

	//Add the lines now
	for (auto childNode : m_childNodes) {
		if (childNode) {
			Vec2 childTopPortPos;
			float childTopPortRadius = 0.0f;
			childNode->GetTopPortDiscData(childTopPortPos, childTopPortRadius);
			AddVertsForLineSegment2D(m_outlineVerts, bottomPortPos, childTopPortPos, TREEEDITOR_CONNECTIONLINETHICKNESS, Rgba8::WHITE);
		}
	}

	Renderer& renderer = m_editor->GetRefToRenderer();
	if(m_outlineVBO == nullptr){
		m_outlineVBO = renderer.CreateVertexBuffer(m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), Stringf("Outline VBO for node: %s", m_nodeDisplayStr.c_str()));
	}
	renderer.CopyCPUToGPU(m_outlineVerts.data(), m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_outlineVBO);
}

void BehaviorTreeNode::UpdateTextVertsForMe()
{
	m_textVerts.clear();
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - m_dimensions.y, m_posInEditor.x + m_dimensions.x, m_posInEditor.y);
	AABB2 titleBarAABB(Vec2(treeNodeAABB.m_mins.x, treeNodeAABB.m_maxs.y - m_initialTitleOutlineDimsY), treeNodeAABB.m_maxs);

	Renderer& renderer = m_editor->GetRefToRenderer();
	BitmapFont* bitmapFont = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	GUARANTEE_OR_DIE(bitmapFont != nullptr, "Check BitmapFont name");
	bitmapFont->AddVertsForTextInBox2D(m_textVerts, titleBarAABB, 30.0f, m_nodeDisplayStr, Rgba8::WHITE, FONT_ASPECT);

	if (m_isSetupValidWhenLatestValidityCheck == false) {
		bitmapFont->AddVertsForTextInBox2D(m_textVerts, treeNodeAABB.GetBoxWithin(Vec2(0.0f, 1.0f), Vec2(1.0f, 1.2f)), 15.0f, "Setup Invalid!", Rgba8::WHITE, 0.8f);
	}

	if (m_textVBO == nullptr) {
		m_textVBO = renderer.CreateVertexBuffer(m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), Stringf("Text VBO for node: %s", m_nodeDisplayStr.c_str()));
	}
	renderer.CopyCPUToGPU(m_textVerts.data(), m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
}

void BehaviorTreeNode::UpdateTickFlowIndicatorVertsForMe()
{
	if (m_childNodes.size() == 0)
		return;

	m_tickFlowVerts.clear();

	if (m_tickFlowStopwatch == nullptr) {
		m_tickFlowStopwatch = new Stopwatch(&m_editor->GetClock(), m_totalTimeForFlowCircleToMove);
		m_tickFlowStopwatch->Start();
	}

	while (m_tickFlowStopwatch->DecrementDurationIfElapsed()) {
		//Just so the timer is reset
	}

	GUARANTEE_OR_DIE(m_lastTickedChild != nullptr, "Last Ticked Child should NOT be a nullptr if it has children!");

	Vec2 bottomPortLoc;
	float dummyFloat;
	GetBottomPortDiscData(bottomPortLoc, dummyFloat);

	Vec2 childTopPortLoc;
	m_lastTickedChild->GetTopPortDiscData(childTopPortLoc, dummyFloat);

	Vec2 thisFrameFlowCircleLoc = Lerp(bottomPortLoc, childTopPortLoc, m_tickFlowStopwatch->GetElapsedFraction());
	AddVertsForDisc2D(m_tickFlowVerts, thisFrameFlowCircleLoc, m_flowCircleRadius, Rgba8::WHITE);
	
	Renderer& renderer = m_editor->GetRefToRenderer();
	if (m_tickFlowVBO == nullptr) {
		m_tickFlowVBO = renderer.CreateVertexBuffer(m_tickFlowVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), Stringf("TickFlow VBO for node: %s", m_nodeDisplayStr.c_str()));;
	}
	renderer.CopyCPUToGPU(m_tickFlowVerts.data(), m_tickFlowVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_tickFlowVBO);
}

void BehaviorTreeNode::AlertTickStopped()
{
}

void BehaviorTreeNode::DeleteTickFlowVBO()
{
	delete m_tickFlowVBO;
	m_tickFlowVBO = nullptr;
}

void BehaviorTreeNode::AddNodeToChildren(BehaviorTreeNode& treeNode)
{
	m_childNodes.push_back(&treeNode);
	treeNode.m_parentNode = this;
}

BehaviorTreeNode* BehaviorTreeNode::RemoveNodeFromChildren(BehaviorTreeNode& treeNode)
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

void BehaviorTreeNode::GetTopPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - m_dimensions.y, m_posInEditor.x + m_dimensions.x, m_posInEditor.y);
	out_portDiscCenter = treeNodeAABB.GetPointAtUV(Vec2(0.5f, 1.0f));
	out_discRadius = TREENODE_PORTRADIUS;
}

void BehaviorTreeNode::GetBottomPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - m_dimensions.y, m_posInEditor.x + m_dimensions.x, m_posInEditor.y);
	out_portDiscCenter = treeNodeAABB.GetPointAtUV(Vec2(0.5f, 0.0f));
	out_discRadius = TREENODE_PORTRADIUS;
}

bool BehaviorTreeNode::IsRootNode() const
{
	return m_isRootNode;
}

bool BehaviorTreeNode::IsCursorInside(const Vec2& cursorPos) const
{
	AABB2 treeNodeAABB(m_posInEditor.x, m_posInEditor.y - m_dimensions.y, m_posInEditor.x + m_dimensions.x, m_posInEditor.y);
	return IsPointInsideAABB2D(cursorPos, treeNodeAABB);
}

bool BehaviorTreeNode::IsDeletable() const
{
	return m_isDeletable;
}

bool BehaviorTreeNode::IsMovable() const
{
	return m_isMovable;
}

bool BehaviorTreeNode::IsNodeAChild(const BehaviorTreeNode& childCandidate) const
{
	for (auto node : m_childNodes) {
		if (node == &childCandidate) {
			return true;
		}
	}
	return false;
}

BehaviorTreeNode* BehaviorTreeNode::GetParentNode() const
{
	return m_parentNode;
}

IntRange BehaviorTreeNode::GetChildNumRange() const
{
	return m_childNumRange;
}

int BehaviorTreeNode::GetNumChildNodes() const
{
	return (int)m_childNodes.size();
}

AABB2 BehaviorTreeNode::GetAABB2() const
{
	return AABB2(m_posInEditor.x, m_posInEditor.y - m_dimensions.y, m_posInEditor.x + m_dimensions.x, m_posInEditor.y);
}

void BehaviorTreeNode::SetBehaviorTreeEditor(const BehaviorTreeEditor& editor)
{
	m_editor = &editor;
}

const BehaviorTreeEditor* BehaviorTreeNode::GetConstPtrToEditor() const
{
	return m_editor;
}

void BehaviorTreeNode::ReorderChildNodes()
{
	if (m_childNodes.size() == 0)
		return;

	m_childNodes.sort([](const BehaviorTreeNode* a, const BehaviorTreeNode* b) -> bool {
		// Compare the x-position of the nodes
		return a->m_posInEditor.x < b->m_posInEditor.x;
	});
}

void BehaviorTreeNode::UpdateDimensionsRequestedFromComponent(const Vec2& dim, const NodeHelperComponent& componentThatRequestedThis)
{
	componentThatRequestedThis;
	m_dimensions = dim;
}

Vec2 BehaviorTreeNode::GetInitialDims() const
{
	return m_initialDimensions;
}

Vec2 BehaviorTreeNode::GetPosInEditor() const
{
	return m_posInEditor;
}

AABB2 BehaviorTreeNode::GetAABB2ForInitialDims() const
{
	return AABB2(Vec2(m_posInEditor.x, m_posInEditor.y - m_initialDimensions.y), Vec2(m_posInEditor.x + m_initialDimensions.x, m_posInEditor.y));
}

void BehaviorTreeNode::SetVNManager(VisualNovelManager& manager)
{
	m_manager = &manager;
}

BehaviorTreeNode* BehaviorTreeNode::GetLastTickedChild() const
{
	return m_lastTickedChild;
}

void BehaviorTreeNode::RenderInEditor() const
{
	Renderer& renderer = m_editor->GetRefToRenderer();
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

	if (m_tickFlowVBO && m_editor->IsTicking()) {
		renderer.BindShader(nullptr);
		renderer.BindTexture(nullptr);
		renderer.DrawVertexBuffer(m_tickFlowVBO, (int)m_tickFlowVerts.size());
	}

	std::vector<NodeHelperComponent*> activeComponents;
	for (auto helperComponent : m_helperComponents) {
		if (helperComponent->IsActive() == false) {
			helperComponent->RenderInEditor();
		}
		else {
			activeComponents.push_back(helperComponent);
		}
	}
	for (auto activeHelperComponent : activeComponents) {
		activeHelperComponent->RenderInEditor();
	}
}

std::map<std::string, std::string> BehaviorTreeNode::GetAndSetXMLAttributes()
{
	m_attributeKeyValues["position"] = Stringf("%.2f,%.2f", m_posInEditor.x, m_posInEditor.y);
	return m_attributeKeyValues;
}

void BehaviorTreeNode::UpdateInEditor()
{
	if (m_isSetupValidWhenLatestValidityCheck == false)
		CheckSetupValidity();

	for (auto helperComponent : m_helperComponents) {
		helperComponent->UpdateInEditor();
	}
	UpdateTextVertsForMe();

	std::vector<BehaviorTreeNode*> selectedNodes = m_editor->GetSelectedNodes();
	auto it = std::find(selectedNodes.begin(), selectedNodes.end(), this);
	UpdateOutlineVertsForMe(m_editor->GetCursorPosInEditor(), it != selectedNodes.end());
}